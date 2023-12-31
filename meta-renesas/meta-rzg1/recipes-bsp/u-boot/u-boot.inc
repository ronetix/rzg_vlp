DESCRIPTION = "U-Boot - the Universal Boot Loader"
HOMEPAGE = "http://www.denx.de/wiki/U-Boot/WebHome"
SECTION = "bootloaders"
PROVIDES = "virtual/bootloader"

inherit deploy

EXTRA_OEMAKE = 'CROSS_COMPILE=${TARGET_PREFIX} CC="${TARGET_PREFIX}gcc ${TOOLCHAIN_OPTIONS}"'

python () {
    if not d.getVar("UBOOT_MACHINE", True):
        PN = d.getVar("PN", True)
        FILE = os.path.basename(d.getVar("FILE", True))
        bb.debug(1, "To build %s, see %s for instructions on \
            setting up your machine config" % (PN, FILE))
        raise bb.parse.SkipPackage("UBOOT_MACHINE is not set in the %s machine configuration." % d.getVar("MACHINE", True))
}

# Some versions of u-boot use .bin and others use .img.  By default use .bin
# but enable individual recipes to change this value.
UBOOT_SUFFIX ?= "bin"
UBOOT_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.${UBOOT_SUFFIX}"
UBOOT_BINARY ?= "u-boot.${UBOOT_SUFFIX}"
UBOOT_SYMLINK ?= "u-boot-${MACHINE}.${UBOOT_SUFFIX}"
UBOOT_MAKE_TARGET ?= "all"

# MiniMonitor requires u-boot.srec
UBOOT_SREC ?= "u-boot.srec"
UBOOT_SREC_SYMLINK ?= "u-boot-${MACHINE}.srec"
UBOOT_SREC_IMAGE ?= "u-boot-${MACHINE}-${PV}-${PR}.srec"

# Some versions of u-boot build an SPL (Second Program Loader) image that
# should be packaged along with the u-boot binary as well as placed in the
# deploy directory.  For those versions they can set the following variables
# to allow packaging the SPL.
SPL_BINARY ?= ""
SPL_IMAGE ?= "${SPL_BINARY}-${MACHINE}-${PV}-${PR}"
SPL_SYMLINK ?= "${SPL_BINARY}-${MACHINE}"
PACKAGE_ARCH = "${MACHINE_ARCH}"

do_compile () {
    if [ "${@bb.utils.contains('DISTRO_FEATURES', 'ld-is-gold', 'ld-is-gold', '', d)}" = "ld-is-gold" ] ; then
        sed -i 's/$(CROSS_COMPILE)ld$/$(CROSS_COMPILE)ld.bfd/g' config.mk
    fi

    unset LDFLAGS
    unset CFLAGS
    unset CPPFLAGS
    export LC_ALL="C"
    oe_runmake ${UBOOT_MACHINE}
    oe_runmake ${UBOOT_MAKE_TARGET}
}

do_install () {
    install -d ${D}/boot
    install ${S}/${UBOOT_BINARY} ${D}/boot/${UBOOT_IMAGE}
    ln -sf ${UBOOT_IMAGE} ${D}/boot/${UBOOT_BINARY}

    if [ -e ${WORKDIR}/fw_env.config ] ; then
        install -d ${D}${sysconfdir}
        install -m 644 ${WORKDIR}/fw_env.config ${D}${sysconfdir}/fw_env.config
    fi

    if [ "x${SPL_BINARY}" != "x" ] ; then
        install ${S}/${SPL_BINARY} ${D}/boot/${SPL_IMAGE}
        ln -sf ${SPL_IMAGE} ${D}/boot/${SPL_BINARY}
    fi
}

FILES_${PN} = "/boot ${sysconfdir}"
FILESPATH =. "${FILE_DIRNAME}/u-boot-git/${MACHINE}:"

do_deploy () {
    install -d ${DEPLOYDIR}
    install ${S}/${UBOOT_BINARY} ${DEPLOYDIR}/${UBOOT_IMAGE}

    cd ${DEPLOYDIR}
    rm -f ${UBOOT_BINARY} ${UBOOT_SYMLINK}
    ln -sf ${UBOOT_IMAGE} ${UBOOT_SYMLINK}
    ln -sf ${UBOOT_IMAGE} ${UBOOT_BINARY}

    if [ "x${SPL_BINARY}" != "x" ] ; then
        install ${S}/${SPL_BINARY} ${DEPLOYDIR}/${SPL_IMAGE}
        rm -f ${DEPLOYDIR}/${SPL_BINARY} ${DEPLOYDIR}/${SPL_SYMLINK}
        ln -sf ${SPL_IMAGE} ${DEPLOYDIR}/${SPL_BINARY}
        ln -sf ${SPL_IMAGE} ${DEPLOYDIR}/${SPL_SYMLINK}
    fi
}

do_deploy_append() {
    install ${S}/${UBOOT_SREC} ${DEPLOYDIR}/${UBOOT_SREC_IMAGE}

    cd ${DEPLOYDIR}
    rm -f ${UBOOT_SREC} ${UBOOT_SREC_SYMLINK}
    ln -sf ${UBOOT_SREC_IMAGE} ${UBOOT_SREC}
    ln -sf ${UBOOT_SREC_IMAGE} ${UBOOT_SREC_SYMLINK}
}

addtask deploy before do_build after do_compile

# Let Yocto build u-boot in machine dir to prevent conflict between boards
PACKAGE_ARCH = "${MACHINE_ARCH}"
