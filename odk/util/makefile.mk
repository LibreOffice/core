PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: makefile.pmk
# ------------------------------------------------------------------

all: zipit 

.IF "$(BUILD_SOSL)"==""
# for SUN build without minor

zipit .SETDIR=$(DESTDIR)$/.. .PHONY:
    .IF "$(OS)"=="WNT"
        +zip -ur $(ODKNAME).zip $(ODKNAME)
    .ELIF "$(GUI)"=="UNX"
# 		tar does not properly support update
        +tar cf - $(ODKNAME) | gzip - > $(ODKNAME).tar.gz
    .ENDIF

.ELSE
# for OpenOffice build with minor 

zipit .SETDIR=$(DESTDIR)$/.. .PHONY:
    .IF "$(OS)"=="WNT"
        +zip -ur $(PRODUCT_NAME).zip $(PRODUCT_NAME)
    .ELIF "$(GUI)"=="UNX"
# 		tar does not properly support update
        +tar cf - $(PRODUCT_NAME) | gzip - > $(PRODUCT_NAME).tar.gz
    .ENDIF

.ENDIF



