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
# $Revision: 1.9 $
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

PRJNAME=canvas
TARGET=vclcanvas
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Common ----------------------------------------------------------

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# Disable optimization for SunCC SPARC
.IF "$(OS)$(CPU)"=="SOLARISS" && "$(COM)"!="GCC"
NOOPTFILES = $(SLO)$/canvashelper_texturefill.obj 
.ENDIF

SLOFILES =	$(SLO)$/backbuffer.obj \
            $(SLO)$/bitmapbackbuffer.obj \
            $(SLO)$/cachedbitmap.obj \
            $(SLO)$/canvasbitmap.obj \
            $(SLO)$/canvasbitmaphelper.obj \
            $(SLO)$/canvascustomsprite.obj \
            $(SLO)$/canvasfont.obj \
            $(SLO)$/canvashelper.obj \
            $(SLO)$/canvashelper_texturefill.obj \
            $(SLO)$/devicehelper.obj \
            $(SLO)$/impltools.obj \
            $(SLO)$/spritecanvas.obj \
            $(SLO)$/spritecanvashelper.obj \
            $(SLO)$/spritehelper.obj \
            $(SLO)$/textlayout.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(TOOLSLIB) $(TKLIB) $(CPPULIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(GOODIESLIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# ==========================================================================

.INCLUDE :	target.mk
