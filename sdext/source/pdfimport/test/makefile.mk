#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
        $(TESTSHL2LIB)           \
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
        $(TESTSHL2LIB)           \
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
        $(TESTSHL2LIB)           \
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

#ALLTAR : $(MISC)$/pdfi_unittest_succeeded
