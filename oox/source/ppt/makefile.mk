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

PRJNAME=oox
TARGET=ppt
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/animationspersist.obj \
        $(SLO)$/animationtypes.obj \
        $(SLO)$/animvariantcontext.obj \
        $(SLO)$/backgroundproperties.obj\
        $(SLO)$/buildlistcontext.obj \
        $(SLO)$/commonbehaviorcontext.obj \
        $(SLO)$/commontimenodecontext.obj \
        $(SLO)$/conditioncontext.obj \
        $(SLO)$/customshowlistcontext.obj \
        $(SLO)$/dgmimport.obj\
        $(SLO)$/dgmlayout.obj\
        $(SLO)$/headerfootercontext.obj \
        $(SLO)$/layoutfragmenthandler.obj\
        $(SLO)$/pptfilterhelpers.obj\
        $(SLO)$/pptgraphicshapecontext.obj \
        $(SLO)$/pptimport.obj\
        $(SLO)$/pptshape.obj \
        $(SLO)$/pptshapecontext.obj \
        $(SLO)$/pptshapegroupcontext.obj \
        $(SLO)$/pptshapepropertiescontext.obj \
        $(SLO)$/presentationfragmenthandler.obj\
        $(SLO)$/slidefragmenthandler.obj\
        $(SLO)$/slidemastertextstylescontext.obj \
        $(SLO)$/slidepersist.obj\
        $(SLO)$/slidetimingcontext.obj\
        $(SLO)$/slidetransition.obj\
        $(SLO)$/slidetransitioncontext.obj\
        $(SLO)$/soundactioncontext.obj \
        $(SLO)$/timeanimvaluecontext.obj \
        $(SLO)$/timenode.obj\
        $(SLO)$/timenodelistcontext.obj \
        $(SLO)$/timetargetelementcontext.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
