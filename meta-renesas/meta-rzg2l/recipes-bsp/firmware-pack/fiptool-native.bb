LIC_FILES_CHKSUM = "file://docs/license.rst;md5=b2c740efedc159745b9b31f88ff03dde"

require fiptool-native.inc

URL = "git://github.com/ronetix/arm-trusted-firmware.git"

BRANCH = "v2.9/rz"
SRCREV = "215afc594ae3ebdb152840b090738e0d93807d81"

SRC_URI = "${URL};protocol=https;branch=${BRANCH}"

PV = "2.9+git${SRCPV}"
PR = "r1"
