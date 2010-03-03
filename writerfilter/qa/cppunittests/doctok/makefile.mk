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
TARGET=test-doctok
# this is removed at the moment because we need some enhancements
# TESTDIR=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SYSTEM_CPPUNIT)"=="YES"
CFLAGS+= $(CPPUNIT_CFLAGS)
.ENDIF

# BEGIN ----------------------------------------------------------------
# auto generated Target:doctok by codegen.pl
SHL1OBJS=  \
       $(SLO)$/testdoctok.obj

SHL1TARGET= testdoctok
SHL1STDLIBS=\
   $(SALLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
     $(TESTSHL2LIB)    \
   $(CPPUNITLIB)

SHL1LIBS=\
    $(SLB)$/doctok.lib
SHL1DEPS= \
    $(SHL1LIBS)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP= export.map
# auto generated Target:doctok
# END ------------------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
