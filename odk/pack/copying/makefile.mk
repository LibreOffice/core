PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

IDLDIRLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type d -print))}

DIRLIST = \
    $(DESTDIREXAMPLES)$/officeclient 	\
    {$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLDIRLIST))}

EXAMPLESLIST= \
    $(DESTDIREXAMPLES)$/officeclient$/officeclient.cxx \
    $(DESTDIREXAMPLES)$/officeclient$/README           \
    $(DESTDIREXAMPLES)$/officeclient$/makefile.mk      \
    $(DESTDIREXAMPLES)$/officeclient$/Makefile         \
    $(DESTDIREXAMPLES)$/officeclient$/exports.dxp      \

IDLLIST={$(subst,/,$/ $(shell $(FIND) $(IDLOUT) -type f -print))}
DESTIDLLIST={$(subst,$(IDLOUT),$(DESTDIRIDL) $(IDLLIST))}

all : 	\
    remove_dk \
    $(DIRLIST) \
    $(EXAMPLESLIST) \
    $(DESTIDLLIST)  \
    $(DESTDIRBIN)$/applicat.rdb  \
    $(DESTDIR)$/settings$/dk.mk

$(DIRLIST) :
     -$(MKDIRHIER) 	$@

$(DESTDIREXAMPLES)$/% : $(PRJ)$/examples$/cpp$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRBIN)$/applicat.rdb : $(BINOUT)$/applicat.rdb 
    $(GNUCOPY) $(BINOUT)$/applicat.rdb $@

$(DESTDIR)$/settings$/dk.mk : $(PRJ)$/util$/dk.mk
    +-rm -f $@ >& $(NULLDEV)
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $(PRJ)$/util$/dk.mk $(MY_TEXTCOPY_TARGETPRE) $@

$(DESTDIRIDL)$/% : $(IDLOUT)$/%
    +-rm -f $@
    $(MY_TEXTCOPY) $(MY_TEXTCOPY_SOURCEPRE) $? $(MY_TEXTCOPY_TARGETPRE) $@

remove_dk : 
    +-$(RM)  $(DESTDIRBIN)$/udkapi.rdb >& $(NULLDEV)
