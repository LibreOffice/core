PRJ=..$/..$/..



PRJNAME=extras
TARGET=gallerywwwgraf

.INCLUDE : settings.mk

.INCLUDE : $(PRJ)$/util$/target.pmk



ZIP1TARGET      = $(GALLERY_WWWGRAF_TARGET)

ZIP1LIST        = * -x makefile.*



.INCLUDE : target.mk



