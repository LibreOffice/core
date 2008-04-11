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
# $Revision: 1.6 $
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

PRJ=..$/..$/..$/..
PRJNAME=shell
TARGET=util
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-D_WIN32_IE=0x501

CFLAGS_X64+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS_X64+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS_X64+=-D_WIN32_IE=0x501

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/dbgmacros.obj\
    $(SLO)$/fileextensions.obj\
    $(SLO)$/registry.obj\
    $(SLO)$/utilities.obj\
    $(SLO)$/iso8601_converter.obj

SLOFILES_X64=$(SLO_X64)$/dbgmacros.obj\
    $(SLO_X64)$/fileextensions.obj\
    $(SLO_X64)$/registry.obj\
    $(SLO_X64)$/utilities.obj\
    $(SLO_X64)$/iso8601_converter.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_wntx64.mk

