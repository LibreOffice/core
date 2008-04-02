#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2008-04-02 09:45:37 $
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
TARGETTYPE=GUI
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
            $(SLO)$/cairo_spritecanvas.obj \
            $(SLO)$/cairo_spritecanvashelper.obj \
            $(SLO)$/cairo_spritehelper.obj \
            $(SLO)$/cairo_textlayout.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(TOOLSLIB)

.IF "$(GUI)"=="UNX" 

.IF "$(SYSTEM_CAIRO)" == "YES"
SHL1STDLIBS+= $(CAIRO_LIBS)
.ELSE
SHL1STDLIBS+= -lcairo -lpixman-1
.ENDIF

.IF "$(GUIBASE)"=="aqua"
# native Mac OS X (Quartz)
SLOFILES+= $(SLO)$/cairo_quartz_cairo.obj
OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions
CFLAGSCXX+=$(OBJCXXFLAGS)
.ELSE
# Xlib
SLOFILES+= $(SLO)$/cairo_xlib_cairo.obj \
           $(SLO)$/cairo_xlib_helper.obj
SHL1STDLIBS+= -lX11 -lXrender
.ENDIF

.ELSE    # "$(GUI)"=="UNX" 

.IF "$(GUI)"=="WNT"
SLOFILES+= $(SLO)$/cairo_win32_cairo.obj
.IF "$(COM)"=="GCC"
SHL1STDLIBS+= -lcairo -lgdi32 -lmsimg32
.ELSE
#We build cairo and pixman as separate (static) libs as I couldn't be
#bothered to dig into the obscure makefile.mk stuff enough to combine
#them into one as is normally done.
SHL1STDLIBS+= cairo.lib pixman.lib gdi32.lib 
.ENDIF
.ENDIF

.ENDIF   # "$(GUI)"=="UNX" 

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=exports.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp


# ==========================================================================

.INCLUDE :	target.mk
