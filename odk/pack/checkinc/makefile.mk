PRJ=..$/..
PRJNAME=odk
TARGET=checkinc

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=..$/misc$/$(TARGET).txt

all : checkit

checkit :
    diff -r $(DESTDIRINC)$/osl   $(SOLARINCDIR)$/osl |& $(PERL) $(PRJ)$/util$/checkdiff.pl
       diff -r $(DESTDIRINC)$/rtl   $(SOLARINCDIR)$/rtl |& $(PERL) $(PRJ)$/util$/checkdiff.pl
       diff -r $(DESTDIRINC)$/sal   $(SOLARINCDIR)$/sal |& $(PERL) $(PRJ)$/util$/checkdiff.pl
#   	diff -r $(DESTDIRINC)$/stl   $(SOLARINCDIR)$/stl |& $(PERL) $(PRJ)$/util$/checkdiff.pl
       diff -r $(DESTDIRINC)$/store $(SOLARINCDIR)$/store |& $(PERL) $(PRJ)$/util$/checkdiff.pl
       diff -r $(DESTDIRINC)$/vos   $(SOLARINCDIR)$/vos |& $(PERL) $(PRJ)$/util$/checkdiff.pl
