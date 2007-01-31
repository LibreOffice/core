#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2007-01-31 08:35:57 $
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

PRJNAME=canvas
TARGET=cairocanvas
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Nothing to do if we're compiling with --disable-cairo -----------
.IF "$(ENABLE_CAIRO)" != "TRUE"
@all:
    @echo "Building without cairo support..."
.ENDIF

# --- Common ----------------------------------------------------------

.IF "$(GUI)"=="UNX"

.IF "$(SYSTEM_CAIRO)" == "YES"
CFLAGS+=$(CAIRO_CFLAGS)
.ELSE
CFLAGS+=-I$(SOLARINCDIR)/cairo
.ENDIF

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

SLOFILES =	$(SLO)$/cairo_cachedbitmap.obj \
            $(SLO)$/cairo_cairo.obj \
            $(SLO)$/cairo_canvasbitmap.obj \
            $(SLO)$/cairo_canvascustomsprite.obj \
            $(SLO)$/cairo_canvasfont.obj \
            $(SLO)$/cairo_canvashelper.obj \
            $(SLO)$/cairo_canvashelper_text.obj \
            $(SLO)$/cairo_devicehelper.obj \
            $(SLO)$/cairo_helper.obj \
            $(SLO)$/cairo_spritecanvas.obj \
            $(SLO)$/cairo_spritecanvashelper.obj \
            $(SLO)$/cairo_spritehelper.obj \
            $(SLO)$/cairo_textlayout.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(TOOLSLIB)

.IF "$(SYSTEM_CAIRO)" == "YES"
SHL1STDLIBS+= $(CAIRO_LIBS) -lX11 -lXrender
.ELSE
SHL1STDLIBS+= -lcairo -lX11 -lXrender
.ENDIF

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

# ==========================================================================

.INCLUDE :	target.mk
