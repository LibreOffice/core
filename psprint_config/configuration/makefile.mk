PRJ=..
PRJNAME=psprint_config
TARGET=unxfontspsprintgeneral

.INCLUDE : settings.mk

ZIP1TARGET      = fontunxpsprint
ZIP1LIST        = * -x makefile.* -x delzip



.INCLUDE : target.mk

