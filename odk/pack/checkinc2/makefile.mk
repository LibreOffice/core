PRJ=..$/..
PRJNAME=odk
TARGET=checkinc2

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=$(MISC)$/$(TARGET).txt

all : checkit

checkit :
    +@echo "" > $(ODKCHECKFILE)
# THE PERL SCRIPT DELETES THE CHECK FILE, WHEN AN ERROR OCCURS
    -diff -br $(DESTDIRINC)$/typelib  $(SOLARINCDIR)$/typelib 	|& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/uno   	$(SOLARINCDIR)$/uno 		|& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/bridges  $(SOLARINCDIR)$/bridges 	|& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/cppu   	$(SOLARINCDIR)$/cppu 		|& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/com$/sun$/star$/uno   	$(SOLARINCDIR)$/com$/sun$/star$/uno 	|& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
    -diff -br $(DESTDIRINC)$/cppuhelper 		$(SOLARINCDIR)$/cppuhelper 		|& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
# RAISE AN ERROR WHEN TAG FILE IS NOT THERE ANYMORE
    cat $(ODKCHECKFILE)
