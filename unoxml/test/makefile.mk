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
        $(TESTSHL2LIB)    \
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
