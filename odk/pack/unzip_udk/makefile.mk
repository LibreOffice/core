PRJ=..$/..
PRJNAME=odk
TARGET=unzip_udk

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

.IF "$(GUI)"=="WNT"
UDKPATH=$(UDKZIPPATH)$/$(UDKZIPPREFIX).zip
ODKDOCPATH=$(UDKZIPPATH)$/$(ODKDOCNAME).zip
.ELSE
UDKPATH=$(UDKZIPPATH)$/$(UDKZIPPREFIX).tar.gz
ODKDOCPATH=$(UDKZIPPATH)$/$(ODKDOCNAME).tar.gz
.ENDIF


all: ..$/misc$/deltree.txt

..$/misc$/deltree.txt .SETDIR=$(OUT)$/bin : $(UDKPATH) ..$/..$/pack$/unzip_udk$/deltree.txt
# first clean everything
.IF "$(BUILD_SOSL)"==""
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME) >& $(NULLDEV)
.ELSE
    +-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME) >& $(NULLDEV)
.ENDIF
    +-$(MY_DELETE_RECURSIVE) $(UDKNAME) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKZIPPREFIX).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKZIPPREFIX).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKZIPPREFIX).tar >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKDOCNAME).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKDOCNAME).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKDOCNAME).tar >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME).tar >& $(NULLDEV)
.IF "$(GUI)"=="WNT"
    $(GNUCOPY) -p $(UDKPATH) $(UDKZIPPREFIX).zip
    $(GNUCOPY) -p $(ODKDOCPATH) $(ODKDOCNAME).zip
    unzip -q -d . $(UDKZIPPREFIX)
    +-$(RENAME) $(UDKNAME) $(ODKNAME)
    unzip -q -d . $(ODKDOCNAME).zip
.ELSE
    $(GNUCOPY) -p $(UDKPATH) .
    $(GNUCOPY) -p $(ODKDOCPATH) .
    gzip -df < $(UDKZIPPREFIX).tar.gz | tar -xvf -
    +-$(RENAME) $(UDKNAME) $(ODKNAME)
    gzip -df < $(ODKDOCNAME).tar.gz | tar -xvf -
.ENDIF
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME)$/settings$/dk.mk
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME)$/docs$/basic
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME)$/docs$/common$/man
.IF "$(BUILD_SOSL)"!=""
# for OpenOffice build rename to PRODUCT_NAME
    +-$(RENAME) $(ODKNAME) $(PRODUCT_NAME)
.ENDIF
    +-$(MY_DELETE_RECURSIVE) $(UDKZIPPREFIX).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKZIPPREFIX).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKZIPPREFIX).tar >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKDOCNAME).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKDOCNAME).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKDOCNAME).tar >& $(NULLDEV)
    +@echo "" > ..$/misc$/deltree.txt
