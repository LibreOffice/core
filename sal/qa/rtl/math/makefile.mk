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
TARGET=rtl_math
# this is removed at the moment because we need some enhancements
# TESTDIR=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

CFLAGSCXX += $(CPPUNIT_CFLAGS)

#----------------------------------- OStringBuffer -----------------------------------

SHL1OBJS= \
    $(SLO)$/test_rtl_math.obj

SHL1TARGET= rtl_math
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

# --- BEGIN --------------------------------------------------------
SHL2OBJS=  \
    $(SLO)$/rtl_math.obj
SHL2TARGET= rtl_math2
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL2IMPLIB= i$(SHL2TARGET)
DEF2NAME=    $(SHL2TARGET)
SHL2VERSIONMAP = $(PRJ)$/qa$/export.map



# # --- BEGIN --------------------------------------------------------
# LLA: this is an old test, which seems not to work
# sal_setInt64()
# sal_getInt64()
# does not exist.
#
# SHL3OBJS=  \
# 	$(SLO)$/rtl_old_testint64.obj
# SHL3TARGET= rtl_old_testint64
# SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)
#
# SHL3IMPLIB= i$(SHL3TARGET)
# DEF3NAME=    $(SHL3TARGET)
# SHL3VERSIONMAP = $(PRJ)$/qa$/export.map
#

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : $(PRJ)$/qa$/cppunit_local.mk
