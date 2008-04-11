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
# $Revision: 1.13 $
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

PRJNAME=vcl
TARGET=glyphs

ENABLE_EXCEPTIONS=true
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

.IF "$(SYSTEM_FREETYPE)" == "YES"
CFLAGS+=-DSYSTEM_FREETYPE $(FREETYPE_CFLAGS)
.IF "$(USE_FT_EMBOLDEN)" == "YES"
CFLAGS+=-DUSE_FT_EMBOLDEN
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(USE_BUILTIN_RASTERIZER)" != ""
SLOFILES=\
        $(SLO)$/glyphcache.obj		\
        $(SLO)$/gcach_rbmp.obj		\
        $(SLO)$/gcach_layout.obj	\
        $(SLO)$/gcach_ftyp.obj
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

