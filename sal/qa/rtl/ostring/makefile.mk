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
INCPRE+= $(PRJ)$/qa$/inc

PRJNAME=sal
TARGET=qa_rtl_ostring2

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl
SHL1OBJS=  \
    $(SLO)$/rtl_OString2.obj

SHL1TARGET= rtl_OString2
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF2EXPORTFILE= export.exp
SHL1VERSIONMAP= $(PRJ)$/qa$/export.map
# auto generated Target:joblist
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL2OBJS=  \
    $(SLO)$/rtl_str.obj

SHL2TARGET= rtl_str
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL2IMPLIB= i$(SHL2TARGET)
DEF2NAME    =$(SHL2TARGET)
SHL2VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------

# BEGIN ----------------------------------------------------------------
SHL3OBJS=  \
    $(SLO)$/rtl_string.obj

SHL3TARGET= rtl_string
SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL3IMPLIB= i$(SHL3TARGET)
DEF3NAME    =$(SHL3TARGET)
SHL3VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------
#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : $(PRJ)$/qa$/cppunit_local.mk
