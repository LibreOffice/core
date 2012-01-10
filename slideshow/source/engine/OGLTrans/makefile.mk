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



PRJ=..$/..$/..

PRJNAME=slideshow
TARGET=OGLTrans
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :      settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Nothing to do if we're compiling with --disable-opengl -----------
.IF "$(ENABLE_OPENGL)" != "TRUE"
@all:
    @echo "Building without OpenGL Transitions..."
.ENDIF

# --- Common ----------------------------------------------------------
#.IF "$(GUI)"=="UNX"

SLOFILES = \
        $(SLO)$/OGLTrans_TransitionImpl.obj \
        $(SLO)$/OGLTrans_Shaders.obj \
        $(SLO)$/OGLTrans_TransitionerImpl.obj

DLLPRE=
SHL1TARGET=$(TARGET).uno

.IF "$(GUI)"=="UNX"
.IF "$(GUIBASE)"=="aqua"
    SHL1STDLIBS= $(SALLIB) $(VCLLIB) $(CPPULIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(CANVASTOOLSLIB)
    CFLAGSCXX+=$(OBJCXXFLAGS)
.ELSE
    SHL1STDLIBS= $(SALLIB) $(VCLLIB) $(CPPULIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(CANVASTOOLSLIB) -lGL -lGLU -lX11
.ENDIF
.ELSE
    SHL1STDLIBS= $(SALLIB) $(VCLLIB) $(CPPULIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(CANVASTOOLSLIB) opengl32.lib glu32.lib gdi32.lib
.ENDIF
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

#.ENDIF

# ==========================================================================

.INCLUDE :	target.mk

ALLTAR : $(MISC)/ogltrans.component

$(MISC)/ogltrans.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ogltrans.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ogltrans.component
