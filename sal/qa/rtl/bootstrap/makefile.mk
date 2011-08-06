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
TARGET=rtl_bootstrap

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUI)"=="WNT"
BOOTSTRAPINI=testshl2.ini
MY_SCRIPTCAT=cat
.ELSE
BOOTSTRAPINI=testshl2rc
MY_SCRIPTCAT=tr -d "\015" <
.ENDIF

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl
SHL1OBJS=  \
    $(SLO)$/rtl_Bootstrap.obj

SHL1TARGET= rtl_Bootstrap
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------

OBJ2FILES=$(OBJ)$/bootstrap_process.obj
APP2TARGET=bootstrap_process
APP2OBJS=$(OBJ2FILES)
APP2STDLIBS=$(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : $(PRJ)$/qa$/cppunit_local.mk
