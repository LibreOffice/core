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
# $Revision: 1.5.12.1 $
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

PRJ=..$/..$/..$/..

PRJNAME=slideshow
TARGET=OGLTrans
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :      settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Nothing to do if we're compiling with --disable-opengl or if nto the right OS  -----------
.IF "$(ENABLE_OPENGL)" != "TRUE"  || "$(OS)" == "MACOSX"
@all:
    @echo "Building without OpenGL Transitions..."
.ENDIF

# --- Common ----------------------------------------------------------

SLOFILES = \
    $(SLO)$/OGLTrans_TransitionImpl_$(OS).obj \
    $(SLO)$/OGLTrans_Shaders_$(OS).obj \
    $(SLO)$/OGLTrans_TransitionerImpl_$(OS).obj

SHL1TARGET=$(TARGET).uno

SHL1STDLIBS= $(SALLIB) $(VCLLIB) $(CPPULIB) $(CPPUHELPERLIB) $(COMPHELPERLIB) $(CANVASTOOLSLIB)

SHL1STDLIBS+= \
    -lGL \
    -lGLU \
    -lX11

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=../exports.dxp

# ==========================================================================

.INCLUDE :	target.mk
