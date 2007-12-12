#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kz $ $Date: 2007-12-12 13:17:14 $
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

ENABLE_EXCEPTIONS=TRUE
PRJNAME=psprint
TARGET=fontman

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= -I..$/fontsubset
INCDEPN+= -I..$/fontsubset

.IF "$(ENABLE_FONTCONFIG)" != ""
CDEFS += -DENABLE_FONTCONFIG
.ENDIF
.IF "$(SYSTEM_FREETYPE)"=="YES"
CFLAGS+=$(FREETYPE_CFLAGS)
.ENDIF # "$(SYSTEM_FREETYPE)"=="YES"

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"=="aqua"

SLOFILES=\
    $(SLO)$/fontmanager.obj		\
    $(SLO)$/fontcache.obj		\
    $(SLO)$/fontconfig.obj		\
    $(SLO)$/parseAFM.obj

.IF "$(OS)$(CPU)"=="SOLARISI"
NOOPTFILES=$(SLO)$/fontmanager.obj
.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
