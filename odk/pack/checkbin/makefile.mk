PRJ=..$/..
PRJNAME=odk
TARGET=checkbin

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=$(MISC)$/$(TARGET).txt

all: checkit

checkit:
    +@echo "" > $(ODKCHECKFILE)
# THE PERL SCRIPT DELETES THE CHECK FILE, WHEN AN ERROR OCCURS
    -diff -r $(DESTDIRBIN)  $(SOLARBINDIR) |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
     -diff -r $(DESTDIRLIB)  $(SOLARLIBDIR) |& $(PERL) $(PRJ)$/util$/checkdiff.pl $(ODKCHECKFILE)
# RAISE AN ERROR WHEN TAG FILE IS NOT THERE ANYMORE
    cat $(ODKCHECKFILE)
