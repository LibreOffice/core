PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: makefile.pmk
# ------------------------------------------------------------------

.IF "$(BUILD_SOSL)"==""
# for SUN build
ZIPFILE=$(ODKZIPFILE)
TARGZFILE=$(ODKTARGZFILE)
ZIPDIR=$(ODKNAME)
.ELSE
# for OO build
ZIPFILE=$(PRODUCTZIPFILE)
TARGZFILE=$(PRODUCTTARGZFILE)
ZIPDIR=$(PRODUCT_NAME)
.ENDIF

.IF "$(OS)"=="WNT"
all:\
    $(BIN)$/$(ZIPFILE)
.ELSE
all:\
    $(BIN)$/$(TARGZFILE)
.ENDIF


$(BIN)$/$(ZIPFILE) .SETDIR=$(DESTDIR)$/.. .PHONY:
    +zip -ur $(ZIPFILE) $(ZIPDIR)

$(BIN)$/$(TARGZFILE) .SETDIR=$(DESTDIR)$/.. .PHONY:
#	tar does not properly support update
    +tar cf - $(ZIPDIR) | gzip - > $(TARGZFILE)
    



