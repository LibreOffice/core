PRJ=..$/..
PRJNAME=odk
TARGET=checkinc

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=..$/misc$/$(TARGET).txt

all : checkit

checkit .SETDIR=$(DESTDIR)$/.. :
      $(MY_DIRCMP) $(ODKNAME)$/include$/osl $(SOLARINCDIR)$/osl > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/rtl $(SOLARINCDIR)$/rtl > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/sal $(SOLARINCDIR)$/sal > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
#   	$(MY_DIRCMP) $(ODKNAME)$/include$/stl $(SOLARINCDIR)$/stl > $(ODKCHECKFILE)
#   	+$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
#   	+$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/store $(SOLARINCDIR)$/store > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
      $(MY_DIRCMP) $(ODKNAME)$/include$/vos $(SOLARINCDIR)$/vos > $(ODKCHECKFILE)
      +$(PERL) $(CHECKSCRIPT) $(ODKCHECKFILE)
      +$(RM) $(ODKCHECKFILE) >$(NULLDEV)
