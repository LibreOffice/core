#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: hr $ $Date: 2006-10-24 14:24:16 $
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

