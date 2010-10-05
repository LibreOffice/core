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
