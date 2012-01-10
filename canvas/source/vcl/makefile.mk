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
            $(SLO)$/canvas.obj \
            $(SLO)$/canvasbitmap.obj \
            $(SLO)$/canvasbitmaphelper.obj \
            $(SLO)$/canvascustomsprite.obj \
            $(SLO)$/canvasfont.obj \
            $(SLO)$/canvashelper.obj \
            $(SLO)$/canvashelper_texturefill.obj \
            $(SLO)$/devicehelper.obj \
            $(SLO)$/impltools.obj \
            $(SLO)$/services.obj \
            $(SLO)$/spritecanvas.obj \
            $(SLO)$/spritecanvashelper.obj \
            $(SLO)$/spritedevicehelper.obj \
            $(SLO)$/spritehelper.obj \
            $(SLO)$/textlayout.obj \
            $(SLO)$/windowoutdevholder.obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(TOOLSLIB) $(TKLIB) $(CPPULIB) $(SALLIB) $(VCLLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(BASEGFXLIB) $(CANVASTOOLSLIB) $(SVTOOLLIB) $(I18NISOLANGLIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# ==========================================================================

.INCLUDE :	target.mk

ALLTAR : $(MISC)/vclcanvas.component

$(MISC)/vclcanvas.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        vclcanvas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt vclcanvas.component
