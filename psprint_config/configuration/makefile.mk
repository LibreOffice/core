PRJ=..
PRJNAME=psprint_config
TARGET=unxfontspsprintgeneral

.INCLUDE : settings.mk

ZIP1TARGET      = fontunxpsprint
ZIP1LIST        = psprint.conf

.INCLUDE : target.mk

