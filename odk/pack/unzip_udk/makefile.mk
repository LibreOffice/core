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
    +-$(RM) ..$/misc$/unzip_udk_succeeded.txt >& $(NULLDEV)
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
    $(GNUCOPY) -p $(UDKPATH) .
    $(GNUCOPY) -p $(ODKDOCPATH) .
.IF "$(GUI)"=="WNT"
    unzip -q -d . $(UDKZIPPREFIX)
    +-$(RENAME) $(UDKNAME) $(ODKNAME)
    unzip -q -d . $(ODKDOCNAME).zip
.ELSE
    gzip -df < $(UDKZIPPREFIX).tar.gz | tar -xvf -
    +-$(RENAME) $(UDKNAME) $(ODKNAME)
    gzip -df < $(ODKDOCNAME).tar.gz | tar -xvf -
.ENDIF
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
    +$(MY_DELETE_RECURSIVE) $(ODKNAME)$/settings$/dk.mk
    touch ..$/misc$/deltree.txt
