#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..
PRJNAME=sdext
TARGET=tests
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- test lib ------------------------------------------------------

.IF "$(ENABLE_PDFIMPORT)" == "NO"
@all:
        @echo "PDF Import extension disabled."
.ENDIF

CFLAGSCXX += $(CPPUNIT_CFLAGS)

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

SHL1OBJS=  \
    $(SLO)$/tests.obj

SHL1LIBS=\
    $(SLB)$/pdfmisc.lib \
    $(SLB)$/pdfparse.lib \
    $(SLB)$/pdfsax.lib \
    $(SLB)$/pdfparsetree.lib \
    $(SLB)$/pdfodf.lib \
    $(SLB)$/xpdfwrapper.lib \
    $(SLB)$/pdfimport.lib

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(BASEGFXLIB)		\
    $(UNOTOOLSLIB)		\
    $(CANVASTOOLSLIB)	\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPUNITLIB)		\
    $(ZLIB3RDLIB)	    \
    $(CPPULIB)			\
    $(SALLIB)


SHL1IMPLIB= i$(SHL1TARGET)
DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map

# --- pdf2xml binary ------------------------------------------------------

TARGET2=pdf2xml

APP1TARGET=$(TARGET2)
APP1LIBSALCPPRT=
APP1OBJS=$(SLO)$/pdf2xml.obj

APP1LIBS=\
    $(SLB)$/pdfmisc.lib \
    $(SLB)$/pdfparse.lib \
    $(SLB)$/pdfsax.lib \
    $(SLB)$/pdfparsetree.lib \
    $(SLB)$/pdfodf.lib \
    $(SLB)$/xpdfwrapper.lib \
    $(SLB)$/pdfimport.lib

APP1STDLIBS=\
    $(BASEGFXLIB)		\
    $(UNOTOOLSLIB)		\
    $(CANVASTOOLSLIB)	\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPUNITLIB)		\
    $(ZLIB3RDLIB)	    \
    $(CPPULIB)			\
    $(SALLIB)

# --- pdfunzip binary ------------------------------------------------------

TARGET3=pdfunzip

APP2TARGET=$(TARGET3)
APP2LIBSALCPPRT=
APP2OBJS=$(SLO)$/pdfunzip.obj

APP2LIBS=\
    $(SLB)$/pdfmisc.lib \
    $(SLB)$/pdfparse.lib \
    $(SLB)$/pdfsax.lib \
    $(SLB)$/pdfparsetree.lib \
    $(SLB)$/pdfodf.lib \
    $(SLB)$/xpdfwrapper.lib \
    $(SLB)$/pdfimport.lib

APP2STDLIBS=\
    $(BASEGFXLIB)		\
      $(UNOTOOLSLIB)		\
      $(CANVASTOOLSLIB)	\
      $(COMPHELPERLIB)	\
      $(CPPUHELPERLIB)	\
      $(CPPUNITLIB)		\
      $(ZLIB3RDLIB)	    \
      $(CPPULIB)			\
      $(SALLIB)

# --- Targets ------------------------------------------------------

  .INCLUDE : target.mk
  .INCLUDE : _cppunit.mk

# --- Fake uno bootstrap & copy testfile ------------------------

  $(BIN)$/pdfi_unittest_test.pdf : testinput.pdf
      rm -f $@
      $(GNUCOPY) testinput.pdf $@

  $(BIN)$/pdfi_unittest_test.ini : makefile.mk
      rm -f $@
      @echo UNO_SERVICES= > $@
      @echo UNO_TYPES=$(UNOUCRRDB:s/\/\\/) >> $@

# --- Enable testshl2 execution in normal build ------------------------

  $(MISC)$/pdfi_unittest_succeeded : $(SHL1TARGETN) $(BIN)$/pdfi_unittest_test.pdf $(BIN)$/pdfi_unittest_test.ini
          rm -f $(BIN)$/pdfi_unittest_draw.xml
          rm -f $(BIN)$/pdfi_unittest_writer.xml
          @echo ----------------------------------------------------------
          @echo - start unit test on library $(SHL1TARGETN)
          @echo ----------------------------------------------------------
          testshl2 -forward $(BIN)$/ $(SHL1TARGETN)
           $(TOUCH) $@
