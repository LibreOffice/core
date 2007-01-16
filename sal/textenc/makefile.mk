#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: vg $ $Date: 2007-01-16 16:05:26 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
