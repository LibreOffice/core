PRJ=..$/..
PRJNAME=odk
TARGET=checkinc2

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=..$/misc$/$(TARGET).txt

all : checkit

checkit :
    diff -r $(DESTDIRINC)$/typelib  $(SOLARINCDIR)$/typelib 	|& $(PERL) $(PRJ)$/util$/checkdiff.pl
    diff -r $(DESTDIRINC)$/uno   	$(SOLARINCDIR)$/uno 		|& $(PERL) $(PRJ)$/util$/checkdiff.pl
    diff -r $(DESTDIRINC)$/bridges  $(SOLARINCDIR)$/bridges 	|& $(PERL) $(PRJ)$/util$/checkdiff.pl
    diff -r $(DESTDIRINC)$/cppu   	$(SOLARINCDIR)$/cppu 		|& $(PERL) $(PRJ)$/util$/checkdiff.pl
    diff -r $(DESTDIRINC)$/com$/sun$/star$/uno   	$(SOLARINCDIR)$/com$/sun$/star$/uno 	|& $(PERL) $(PRJ)$/util$/checkdiff.pl
    diff -r $(DESTDIRINC)$/cppuhelper 		$(SOLARINCDIR)$/cppuhelper 		|& $(PERL) $(PRJ)$/util$/checkdiff.pl