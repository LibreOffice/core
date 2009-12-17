#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
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
#BOOTSTRAPSCRIPT=bootstrap.bat
BOOTSTRAPINI=testshl2.ini
MY_SCRIPTCAT=cat
.ELSE
#BOOTSTRAPSCRIPT=bootstrap
BOOTSTRAPINI=testshl2rc
MY_SCRIPTCAT=tr -d "\015" <
.ENDIF

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl
SHL1OBJS=  \
    $(SLO)$/rtl_Bootstrap.obj

SHL1TARGET= rtl_Bootstrap
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB) $(TESTSHL2LIB)

SHL1IMPLIB= i$(SHL1TARGET)
# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP= $(PRJ)$/qa$/export.map
# END ------------------------------------------------------------------

OBJ2FILES=$(OBJ)$/bootstrap_process.obj
APP2TARGET=bootstrap_process
APP2OBJS=$(OBJ2FILES)

# .IF "$(GUI)" == "UNX"
# APP2STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP2STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP2STDLIBS=$(SALLIB)
#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
# SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk

