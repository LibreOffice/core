PRJ=..$/..
PRJNAME=odk
TARGET=checkinc

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=$(MISC)$/$(TARGET).txt

all : checkit

checkit :
    +@echo "" > $(ODKCHECKFILE)
# THE PERL SCRIPT DELETES THE CHECK FILE, WHEN AN ERROR OCCURS
    -diff -br $(DESTDIRINC)$/osl   $(SOLARINCDIR)$/osl |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
       -diff -br $(DESTDIRINC)$/rtl   $(SOLARINCDIR)$/rtl |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
       -diff -br $(DESTDIRINC)$/sal   $(SOLARINCDIR)$/sal |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
#   	-diff -br $(DESTDIRINC)$/stl   $(SOLARINCDIR)$/stl |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
       -diff -br $(DESTDIRINC)$/store $(SOLARINCDIR)$/store |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
       -diff -br $(DESTDIRINC)$/vos   $(SOLARINCDIR)$/vos |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
# RAISE AN ERROR WHEN TAG FILE IS NOT THERE ANYMORE
    cat $(ODKCHECKFILE)
