PRJ=..$/..
PRJNAME=odk
TARGET=unzip_udk

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

.IF "$(GUI)"=="WNT"
UDKPATH=$(UDKZIPPATH)$/$(UDKNAME).zip
.ELSE
UDKPATH=$(UDKZIPPATH)$/$(UDKNAME).tar.gz
.ENDIF

all: ..$/misc$/deltree.txt

..$/misc$/deltree.txt .SETDIR=$(OUT)$/bin : $(UDKPATH) ..$/..$/pack$/unzip_udk$/deltree.txt
# first clean everything
    +-$(RM) ..$/misc$/unzip_udk_succeeded.txt >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKNAME) >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKNAME).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKNAME).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(UDKNAME).tar >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME).zip >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME).tar.gz >& $(NULLDEV)
    +-$(MY_DELETE_RECURSIVE) $(ODKNAME).tar >& $(NULLDEV)
    $(GNUCOPY) -p $(UDKPATH) .
.IF "$(GUI)"=="WNT"
    unzip -q -d . $(UDKNAME)
.ELSE
    gzip -df < $(UDKNAME).tar.gz | tar -xvf -
.ENDIF
    +-$(RENAME) $(UDKNAME) 		$(ODKNAME)
    +rm -f $(ODKNAME)$/settings$/dk.mk
    touch ..$/misc$/deltree.txt
