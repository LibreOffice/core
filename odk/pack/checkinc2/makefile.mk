PRJ=..$/..
PRJNAME=odk
TARGET=checkinc2

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=..$/misc$/$(TARGET).txt

all : checkit

checkit .SETDIR=$(DESTDIR)$/.. :
      $(MY_DIRCMP) $(ODKNAME)$/include$/typelib $(SOLARINCDIR)$/typelib > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/uno $(SOLARINCDIR)$/uno > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/bridges $(SOLARINCDIR)$/bridges > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/com$/sun$/star$/uno $(SOLARINCDIR)$/com$/sun$/star$/uno > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/cppu $(SOLARINCDIR)$/cppu > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/cppuhelper $(SOLARINCDIR)$/cppuhelper > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
