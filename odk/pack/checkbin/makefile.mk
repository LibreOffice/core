PRJ=..$/..
PRJNAME=odk
TARGET=checkbin

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=..$/misc$/$(TARGET).txt

all: checkit

checkit .SETDIR=$(DESTDIR)$/.. :
     $(MY_DIRCMP) $(ODKNAME)$/bin $(SOLARBINDIR) > $(ODKCHECKFILE)
     +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
     +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
     $(MY_DIRCMP) $(ODKNAME)$/lib $(SOLARLIBDIR) > $(ODKCHECKFILE)
     +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
     +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
