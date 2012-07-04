#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..

PRJNAME=unoxml
TARGET=tests
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# --- Common ----------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:tests by codegen.pl
SHL1OBJS=  \
    $(SLO)$/domtest.obj

# the following three libs are a bit of a hack: cannot link against
# unoxml here, because not yet delivered (and does not export
# ~anything). Need the functionality to test, so we're linking it in
# statically. Need to keep this in sync with
# source/services/makefile.mk
SHL1LIBS= \
    $(SLB)$/domimpl.lib \
    $(SLB)$/xpathimpl.lib \
    $(SLB)$/eventsimpl.lib

SHL1TARGET= tests
SHL1STDLIBS= \
    $(UCBHELPERLIB) \
    $(LIBXML2LIB) \
    $(TOOLSLIB)	\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPUNITLIB)	 \
    $(CPPULIB)	\
    $(SAXLIB) \
    $(SALLIB)\
    $(EXPATASCII3RDLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
.INCLUDE : _cppunit.mk

# --- Fake uno bootstrap ------------------------

$(BIN)$/unoxml_unittest_test.ini : makefile.mk
    rm -f $@
    @echo UNO_SERVICES= > $@
    @echo UNO_TYPES=$(UNOUCRRDB:s/\/\\/) >> $@

# --- Enable testshl2 execution in normal build ------------------------

$(MISC)$/unoxml_unittest_succeeded : $(SHL1TARGETN) $(BIN)$/unoxml_unittest_test.ini
        @echo ----------------------------------------------------------
        @echo - start unit test on library $(SHL1TARGETN)
        @echo ----------------------------------------------------------
                $(TESTSHL2) -forward $(BIN)$/ -sf $(mktmp ) $(SHL1TARGETN)
        $(TOUCH) $@

ALLTAR : $(MISC)$/unoxml_unittest_succeeded
