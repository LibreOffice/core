#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..

PRJNAME=canvas
TARGET=cairocanvas
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =

# --- Nothing to do if we're compiling with --disable-cairo-canvas -----------
.IF "$(ENABLE_CAIRO_CANVAS)" != "TRUE"
@all:
    @echo "Building without cairo support..."
.ELSE
# --- X11 Mac build currently doesn't work with cairo -----------
.IF "$(OS)" == "MACOSX" && "$(GUIBASE)" == "unx"
@all:   
    @echo "Cannot build cairocanvas with X11..."
.ENDIF
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

.IF "$(GUIBASE)"=="aqua"
# native Mac OS X (Quartz)
SLOFILES+= $(SLO)$/cairo_quartz_cairo.obj
CFLAGSCXX+=$(OBJCXXFLAGS)
.ELSE    # "$(GUIBASE)"=="aqua"

# Xlib
SLOFILES+= $(SLO)$/cairo_xlib_cairo.obj
SHL1STDLIBS+= -lfontconfig -lX11 -lXrender $(FREETYPE_LIBS)
CFLAGS+=$(FREETYPE_CFLAGS)

.ENDIF   # "$(GUIBASE)"=="aqua"

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
