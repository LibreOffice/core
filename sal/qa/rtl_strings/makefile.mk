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
PRJ=..$/..

PRJNAME=sal
TARGET=qa_rtl_strings
# TESTDIR=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:FileBase by codegen.pl
SHL1OBJS=  \
    $(SLO)$/rtl_String_Utils.obj \
    $(SLO)$/rtl_OString.obj

SHL1TARGET= rtl_OString
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

# auto generated Target:FileBase
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:FileBase by codegen.pl
SHL2OBJS=  \
    $(SLO)$/rtl_String_Utils.obj \
    $(SLO)$/rtl_OUString.obj

SHL2TARGET= rtl_OUString
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL2IMPLIB= i$(SHL2TARGET)
# SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME    =$(SHL2TARGET)
# DEF2EXPORTFILE= export.exp
SHL2VERSIONMAP = $(PRJ)$/qa$/export.map

# auto generated Target:FileBase
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
# auto generated Target:FileBase by codegen.pl
SHL3OBJS=  \
    $(SLO)$/rtl_String_Utils.obj \
    $(SLO)$/rtl_OUStringBuffer.obj

SHL3TARGET= rtl_OUStringBuffer
SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL3IMPLIB= i$(SHL3TARGET)
# SHL3DEF=    $(MISC)$/$(SHL3TARGET).def

DEF3NAME    =$(SHL3TARGET)
# DEF3EXPORTFILE= export.exp
SHL3VERSIONMAP = $(PRJ)$/qa$/export.map

# auto generated Target:FileBase
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL4OBJS=  \
    $(SLO)$/rtl_old_teststrbuf.obj

SHL4TARGET= rtl_old_teststrbuf
SHL4STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL4IMPLIB= i$(SHL4TARGET)
DEF4NAME    =$(SHL4TARGET)
SHL4VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL5OBJS=  \
    $(SLO)$/rtl_old_testowstring.obj

SHL5TARGET= rtl_old_testowstring
SHL5STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL5IMPLIB= i$(SHL5TARGET)
DEF5NAME    =$(SHL5TARGET)
SHL5VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL6OBJS=  \
    $(SLO)$/rtl_old_testostring.obj

SHL6TARGET= rtl_old_testostring
SHL6STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL6IMPLIB= i$(SHL6TARGET)
DEF6NAME    =$(SHL6TARGET)
SHL6VERSIONMAP = $(PRJ)$/qa$/export.map

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=\
    $(SHL1OBJS) \
    $(SHL2OBJS) \
    $(SHL3OBJS) \
    $(SHL4OBJS) \
    $(SHL5OBJS) \
    $(SHL6OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : $(PRJ)$/qa$/cppunit_local.mk
