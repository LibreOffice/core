PRJ=..$/..
PRJNAME=psp_config
TARGET=unxfontsxp3ppds

.INCLUDE : settings.mk

ZIP1TARGET      = fontunxppds
ZIP1LIST        = * -x makefile.* -x delzip

.INCLUDE : target.mk

