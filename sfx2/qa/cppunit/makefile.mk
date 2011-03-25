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

.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE

PRJ=../..
PRJNAME=sfx2
TARGET=qa_cppunit

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#building with stlport, but cppunit was not built with stlport
.IF "$(USE_SYSTEM_STL)"!="YES"
.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGSCXX+=-DADAPT_EXT_STL
.ENDIF
.ENDIF

CFLAGSCXX += $(CPPUNIT_CFLAGS)
DLLPRE = # no leading "lib" on .so files

# --- Libs ---------------------------------------------------------

SHL1OBJS=  \
    $(SLO)/test_metadatable.obj \


SHL1STDLIBS= \
     $(CPPUNITLIB) \
     $(SALLIB) \
     $(CPPULIB) \
     $(CPPUHELPERLIB) \
     $(VCLLIB) \
     $(SFXLIB) \


SHL1TARGET= test_metadatable
SHL1RPATH = NONE
SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF= $(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP= version.map

# --- All object files ---------------------------------------------

SLOFILES= \
    $(SHL1OBJS) \


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

.END
