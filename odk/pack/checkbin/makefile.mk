PRJ=..$/..
PRJNAME=odk
TARGET=checkbin

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------
ODKCHECKFILE=..$/misc$/$(TARGET).txt

all: checkit

checkit:
    diff -r $(DESTDIRBIN)  $(SOLARBINDIR) |& $(PERL) $(PRJ)$/util$/checkdiff.pl
     diff -r $(DESTDIRLIB)  $(SOLARLIBDIR) |& $(PERL) $(PRJ)$/util$/checkdiff.pl
