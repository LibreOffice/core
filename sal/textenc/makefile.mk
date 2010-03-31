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

.INCLUDE: settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

SLOFILES = \
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
    $(SLO)$/tencinfo.obj \
    $(SLO)$/textcvt.obj \
    $(SLO)$/textenc.obj \
    $(SLO)$/unichars.obj

OBJFILES = \
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
    $(OBJ)$/tencinfo.obj \
    $(OBJ)$/textcvt.obj \
    $(OBJ)$/textenc.obj \
    $(OBJ)$/unichars.obj

# Optimization off on Solaris Intel due to internal compiler error; to be
# reevaluated after compiler upgrade:
.IF "$(OS)$(CPU)" == "SOLARISI"

NOOPTFILES = \
    $(OBJ)$/textenc.obj \
    $(SLO)$/textenc.obj

.ENDIF # OS, CPU, SOLARISI

.INCLUDE: target.mk
