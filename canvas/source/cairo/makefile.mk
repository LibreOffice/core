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

PRJ=..$/..

PRJNAME=canvas
TARGET=cairocanvas
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

.IF "$(OS)" == "MACOSX" && "$(GUIBASE)" == "unx"
@all:   
    @echo "Cannot build cairocanvas with X11..."
.ENDIF

.IF "$(OS)" == "ANDROID"
@all:   
    @echo "Cannot build cairocanvas without cairo..."
.ENDIF

# --- Common ----------------------------------------------------------


.IF "$(SYSTEM_CAIRO)" == "YES"
CFLAGS+=$(CAIRO_CFLAGS)
.ELSE
CFLAGS+=-I$(SOLARINCDIR)/cairo
.ENDIF

.IF "$(OS)" == "IOS"
CFLAGS+= -x objective-c++ -fobjc-exceptions -fobjc-abi-version=2 -fobjc-legacy-dispatch -D__IPHONE_OS_VERSION_MIN_REQUIRED=40300
.ENDIF

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF
.IF "$(L10N_framework)"==""
SLOFILES =	$(SLO)$/cairo_cachedbitmap.obj \
            $(SLO)$/cairo_cairo.obj \
            $(SLO)$/cairo_canvas.obj \
            $(SLO)$/cairo_canvasbitmap.obj \
            $(SLO)$/cairo_canvascustomsprite.obj \
            $(SLO)$/cairo_canvasfont.obj \
            $(SLO)$/cairo_canvashelper.obj \
            $(SLO)$/cairo_canvashelper_text.obj \
            $(SLO)$/cairo_devicehelper.obj \
            $(SLO)$/cairo_services.obj \
            $(SLO)$/cairo_spritecanvas.obj \
            $(SLO)$/cairo_spritecanvashelper.obj \
            $(SLO)$/cairo_spritedevicehelper.obj \
            $(SLO)$/cairo_spritehelper.obj \
            $(SLO)$/cairo_textlayout.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(CPPULIB) $(TKLIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(TOOLSLIB) $(I18NISOLANGLIB)

.IF "$(GUI)"=="UNX" 

.IF "$(SYSTEM_CAIRO)" == "YES"
SHL1STDLIBS+= $(CAIRO_LIBS)
.ELSE
SHL1STDLIBS+= -lcairo -lpixman-1
.ENDIF

.IF "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="cocoatouch"
# native Mac OS X (Quartz) or iOS
SLOFILES+= $(SLO)$/cairo_quartz_cairo.obj
CFLAGSCXX+=$(OBJCXXFLAGS)
.ELSE    # "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="cocoatouch"

# Xlib
SLOFILES+= $(SLO)$/cairo_xlib_cairo.obj
SHL1STDLIBS+= $(FONTCONFIG_LIBS) -lX11 -lXrender $(FREETYPE_LIBS)
CFLAGS+= $(FREETYPE_CFLAGS)

.ENDIF   # "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="cocoatouch"

.ELSE    # "$(GUI)"=="UNX" 

.IF "$(GUI)"=="WNT"
SLOFILES+= $(SLO)$/cairo_win32_cairo.obj
.IF "$(COM)"=="GCC"
SHL1STDLIBS+= -lcairo
.ELSE
SHL1STDLIBS+= cairo.lib
.ENDIF
SHL1STDLIBS+= $(GDI32LIB) $(MSIMG32LIB)
.ENDIF

.ENDIF   # "$(GUI)"=="UNX" 

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF
# ==========================================================================

.INCLUDE :	target.mk

ALLTAR : $(MISC)/cairocanvas.component

$(MISC)/cairocanvas.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt cairocanvas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt cairocanvas.component
