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

PRJNAME=sal
TARGET=qa_osl_process

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:testjob by codegen.pl

.IF "$(GUI)" == "WNT"
    CFLAGS+=/Ob1
.ENDIF

SHL1OBJS=  \
    $(SLO)$/osl_Thread.obj

SHL1TARGET= osl_Thread
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)

# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)

# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

#.IF "$(GUI)" == "WNT"

SHL2OBJS=$(SLO)$/osl_process.obj
SHL2TARGET=osl_process
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL2IMPLIB=i$(SHL2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=export.exp

# END ------------------------------------------------------------------

OBJ3FILES=$(OBJ)$/osl_process_child.obj
APP3TARGET=osl_process_child
APP3OBJS=$(OBJ3FILES)

# .IF "$(GUI)" == "UNX"
# APP3STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP3STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP3STDLIBS=$(SALLIB)

#.ENDIF # "$(GUI)" == "WNT"

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies

.IF "$(GUI)" == "OS2"

SLOFILES=$(SHL2OBJS)

.ELSE

SLOFILES=$(SHL1OBJS) $(SHL2OBJS)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
