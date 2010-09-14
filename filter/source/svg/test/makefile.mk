#*************************************************************************
#
#    OpenOffice.org - a multi-platform office productivity suite
#
#    Author:
#      Fridrich Strba  <fridrich.strba@bluewin.ch>
#      Thorsten Behrens <tbehrens@novell.com>
#
#      Copyright (C) 2008, Novell Inc.
#      Parts copyright 2005 by Sun Microsystems, Inc.
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 3.
#
#*************************************************************************

PRJ=..$/..$/..
PRJNAME=filter
TARGET=tests
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- unit tests ---------------------------------------------------

SHL1OBJS=  \
    $(SLO)$/parsertest.obj

SHL1TARGET= tests
SHL1LIBS= $(SLB)$/svgfilter.lib
SHL1STDLIBS= 	        \
    $(BASEGFXLIB)		\
    $(SVXLIB)			\
    $(SVTOOLLIB)	    \
    $(XMLOFFLIB)		\
    $(BASEGFXLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(SVTOOLLIB)	    \
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)			\
    $(LIBXML)			\
    $(CPPUNITLIB)

# --- svg2xml binary ------------------------------------------------------

TARGET2=svg2odf

APP1TARGET=$(TARGET2)
APP1LIBSALCPPRT=
APP1OBJS= \
    $(SLO)$/odfserializer.obj	\
    $(SLO)$/svg2odf.obj

APP1LIBS=\
    $(SLB)$/svgfilter.lib

APP1STDLIBS=\
    $(BASEGFXLIB)		\
    $(SVXLIB)			\
    $(XMLOFFLIB)		\
    $(BASEGFXLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)			\
    $(LIBXML)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
.INCLUDE : _cppunit.mk

# --- Special ------------------------------------------------------

TESTFILES=\
    anarchist.svg \
    anarchist2.svg \
    Nested.svg

$(MISC)$/%_svgi_unittest_succeeded : $(BIN)$/svg2odf
    rm -f $(MISC)$/$(@:s/_succeeded/.xml/:f)
    $(BIN)$/svg2odf $(@:s/_svgi_unittest_succeeded/.svg/:f) $(MISC)$/$(@:s/_succeeded/.xml/:f) $(BIN)$/svgi_unittest_test.ini
    $(TOUCH) $@

.IF "$(GUI)" == "WNT"
SAXPARSERLIB=$(SOLARBINDIR)$/sax.uno$(DLLPOST)
UNOXMLLIB=$(SOLARBINDIR)$/$(DLLPRE)unoxml$(OFFICEUPD)$(DLLPOSTFIX)$(DLLPOST)
.ELSE
SAXPARSERLIB=$(SOLARLIBDIR)$/sax.uno$(DLLPOST)
UNOXMLLIB=$(SOLARLIBDIR)$/$(DLLPRE)unoxml$(OFFICEUPD)$(DLLPOSTFIX)$(DLLPOST)
.ENDIF

$(BIN)$/unittestservices.rdb : makefile.mk $(SAXPARSERLIB) $(UNOXMLLIB)
    rm -f $@
    $(REGCOMP) -register -r $@ -c $(SAXPARSERLIB)
    $(REGCOMP) -register -r $@ -c $(UNOXMLLIB)

$(BIN)$/svgi_unittest_test.ini : makefile.mk
    rm -f $@
    @echo UNO_SERVICES=$(BIN)$/unittestservices.rdb > $@
    @echo UNO_TYPES=$(UNOUCRRDB:s/\/\\/) >> $@

ALLTAR : $(BIN)$/svgi_unittest_test.ini \
         $(BIN)$/unittestservices.rdb \
         $(foreach,i,$(TESTFILES:s/.svg/_svgi_unittest_succeeded/:f) $(MISC)$/$i)
