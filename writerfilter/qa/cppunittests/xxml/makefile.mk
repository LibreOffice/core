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

PRJNAME=writerfilter
TARGET=test-xxml
# this is removed at the moment because we need some enhancements
# TESTDIR=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGS+= $(CPPUNIT_CFLAGS)
.ENDIF

# BEGIN ----------------------------------------------------------------
# auto generated Target:testjob by codegen.pl

.IF "$(GUI)" == "WNT"
    CFLAGS+=/Ob1
.ENDIF

SHL1OBJS=\
    $(SLO)$/testXXML.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=$(SALLIB) $(TOOLSLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

.IF "$(GUI)" == "WNT"
SHL1STDLIBS+=$(LB)$/iodiapi.lib
.ENDIF
.IF "$(GUI)" == "UNX"
SHL1STDLIBS+=$(LB)$/libodiapi.so
.ENDIF

SHL1IMPLIB= i$(SHL1TARGET)

# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)

# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = export.map


# END ------------------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
