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

PRJ = ..
PRJNAME = sal
TARGET = textenc
LIBTARGET=NO

.INCLUDE: settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

SAL_OBJECTS = \
    $(SLO)$/tencinfo.obj \
    $(SLO)$/textcvt.obj \
    $(SLO)$/textenc.obj

SAL_COMMON_OBJECTS = \
    $(SLO)$/context.obj \
    $(SLO)$/convertbig5hkscs.obj \
    $(SLO)$/converter.obj \
    $(SLO)$/converteuctw.obj \
    $(SLO)$/convertgb18030.obj \
    $(SLO)$/convertiso2022cn.obj \
    $(SLO)$/convertiso2022jp.obj \
    $(SLO)$/convertiso2022kr.obj \
    $(SLO)$/convertsinglebytetobmpunicode.obj \
    $(SLO)$/tcvtbyte.obj \
    $(SLO)$/tcvtmb.obj \
    $(SLO)$/tcvtutf7.obj \
    $(SLO)$/tcvtutf8.obj \
    $(SLO)$/tenchelp.obj \
    $(SLO)$/unichars.obj

SAL_TABLES_OBJECTS = \
    $(SLO)$/tables.obj

SLOFILES = $(SAL_OBJECTS) $(SAL_COMMON_OBJECTS) $(SAL_TABLES_OBJECTS)

SAL_OBJFILES = \
    $(OBJ)$/tencinfo.obj \
    $(OBJ)$/textcvt.obj \
    $(OBJ)$/textenc.obj

SAL_COMMON_OBJFILES = \
    $(OBJ)$/context.obj \
    $(OBJ)$/convertbig5hkscs.obj \
    $(OBJ)$/converter.obj \
    $(OBJ)$/converteuctw.obj \
    $(OBJ)$/convertgb18030.obj \
    $(OBJ)$/convertiso2022cn.obj \
    $(OBJ)$/convertiso2022jp.obj \
    $(OBJ)$/convertiso2022kr.obj \
    $(OBJ)$/convertsinglebytetobmpunicode.obj \
    $(OBJ)$/tcvtbyte.obj \
    $(OBJ)$/tcvtmb.obj \
    $(OBJ)$/tcvtutf7.obj \
    $(OBJ)$/tcvtutf8.obj \
    $(OBJ)$/tenchelp.obj \
    $(OBJ)$/unichars.obj

SAL_TABLES_OBJFILES = \
    $(OBJ)$/tables.obj

OBJFILES = $(SAL_OBJFILES) $(SAL_COMMON_OBJFILES) $(SAL_TABLES_OBJFILES)

# Optimization off on Solaris Intel due to internal compiler error; to be
# reevaluated after compiler upgrade:
.IF "$(OS)$(CPU)" == "SOLARISI"

NOOPTFILES = \
    $(OBJ)$/textenc.obj \
    $(SLO)$/textenc.obj \
    $(OBJ)$/tables.obj \
    $(SLO)$/tables.obj

.ENDIF # OS, CPU, SOLARISI

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES= $(SAL_OBJECTS) $(SAL_COMMON_OBJECTS)
OBJTARGET=$(LB)$/$(TARGET).lib

# be nicer to avoid DLLPRE
.IF "$(GUI)" == "WNT"
CDEFS+=-DPLUGIN_NAME=$(DLLPRE)sal_textenc$(UDK_MAJOR)$(DLLPOST)
.ELSE
CDEFS+=-DPLUGIN_NAME=$(DLLPRE)sal_textenc$(DLLPOST).$(UDK_MAJOR)
.ENDIF
LIB2TARGET= $(SLB)$/textenc_tables.lib
LIB2OBJFILES= $(SAL_TABLES_OBJECTS) $(SAL_COMMON_OBJECTS)

.INCLUDE: target.mk
