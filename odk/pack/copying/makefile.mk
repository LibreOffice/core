PRJ=..$/..
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

DIRLIST = \
    $(DESTDIREXAMPLES)$/officeclient 	

EXAMPLESLIST= \
    $(DESTDIREXAMPLES)$/officeclient$/officeclient.cxx \
    $(DESTDIREXAMPLES)$/officeclient$/README           \
    $(DESTDIREXAMPLES)$/officeclient$/makefile.mk      \
    $(DESTDIREXAMPLES)$/officeclient$/Makefile         \
    $(DESTDIREXAMPLES)$/officeclient$/exports.dxp      \


.IF "$(GUI)"=="UNX"
all : deliver $(DIRLIST) $(EXAMPLESLIST) convertit
.ELSE
all : deliver $(DIRLIST) $(EXAMPLESLIST) 
.ENDIF

$(DIRLIST) : 
     -$(MKDIRHIER) 	$@

$(DESTDIREXAMPLES)$/officeclient$/% : $(PRJ)$/examples$/officeclient$/% $(DIRLIST) $(BIN)$/$(UDKNAME).zip
     $(GNUCOPY) $(PRJ)$/examples$/officeclient$/$(@:f) $@

deliver : 
#------------------------------------------------------------------------------------
#       R D B  F I L E S
#------------------------------------------------------------------------------------
    +-$(RM)  $(DESTDIRBIN)$/udkapi.rdb >& NUL
    +-$(MY_COPY)  $(BINOUT)$/applicat.rdb $(DESTDIRBIN)
# ------------------------------------------------------------------------------------
#       I D L files
# ------------------------------------------------------------------------------------
    +-$(MY_COPY_RECURSIVE) $(IDLOUT) $(DESTDIR)
#----------------------------------------------------------------------
#       S E T T I N G S
#----------------------------------------------------------------------
    +-$(RM) /f $(DESTDIR)$/settings$/dk.mk
    $(MY_COPY) $(PRJ)$/util$/dk.mk $(DESTDIR)$/settings


MKFILES_CONVERT=$(shell $(FIND) . -name "*.mk" -print)
CXFILES_CONVERT=$(shell $(FIND) . -name "*.c*" -print)
HXFILES_CONVERT=$(shell $(FIND) . -name "*.h*" -print)
.IF "$(GUI)"=="UNX"
convertit: rwit dos2unx roit

rwit .SETDIR=$(DESTDIR):
    +echo rwit
    +-chmod 666 $(foreach,file,$(MKFILES_CONVERT) $(file))
    +-chmod 666 $(foreach,file,$(CXFILES_CONVERT) $(file))
    +-chmod 666 $(foreach,file,$(HXFILES_CONVERT) $(file))
dos2unx .SETDIR=$(DESTDIR):
    +echo dos2unx
    +-any2all $(foreach,file,$(MKFILES_CONVERT) $(file)) >$(NULLDEV)
    +-any2all $(foreach,file,$(CXFILES_CONVERT) $(file)) >$(NULLDEV)
    +-any2all $(foreach,file,$(HXFILES_CONVERT) $(file)) >$(NULLDEV)
roit .SETDIR=$(DESTDIR):
    +echo roit
    +-chmod 444 $(foreach,file,$(MKFILES_CONVERT) $(file))
    +-chmod 444 $(foreach,file,$(CXFILES_CONVERT) $(file))
    +-chmod 444 $(foreach,file,$(HXFILES_CONVERT) $(file))
.ENDIF

