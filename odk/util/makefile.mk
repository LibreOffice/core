PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: makefile.pmk
# ------------------------------------------------------------------

.IF "$(OS)"=="WNT"
all:\
    $(BIN)$/$(PRODUCTZIPFILE)
.ELSE
all:\
    $(BIN)$/$(PRODUCTTARGZFILE)
.ENDIF

$(BIN)$/$(PRODUCTZIPFILE) : $(SDK_CONTENT_CHECK_FILES) $(SDKCHECKFLAG)
    +cd $(BIN) && $(WRAPCMD) zip -urq $(PRODUCTZIPFILE) $(PRODUCT_NAME)

$(BIN)$/$(PRODUCTTARGZFILE) : $(SDK_CONTENT_CHECK_FILES) $(SDKCHECKFLAG)
    +-rm -f $@ >& $(NULLDEV)
#	tar does not properly support update
.IF "$(OS)"=="SOLARIS"
#	always use the system tar on Solaris
    +cd $(BIN) && /usr/bin/tar cf - $(PRODUCT_NAME) | gzip - > $(PRODUCTTARGZFILE)
.ELSE
    +cd $(BIN) && tar cf - $(PRODUCT_NAME) | gzip - > $(PRODUCTTARGZFILE)
.ENDIF

    



