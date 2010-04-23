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
PRJNAME=drawinglayer
TARGET=primitive3d
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/baseprimitive3d.obj				\
        $(SLO)$/groupprimitive3d.obj			\
        $(SLO)$/hatchtextureprimitive3d.obj		\
        $(SLO)$/hiddengeometryprimitive3d.obj	\
        $(SLO)$/modifiedcolorprimitive3d.obj	\
        $(SLO)$/polypolygonprimitive3d.obj		\
        $(SLO)$/polygonprimitive3d.obj			\
        $(SLO)$/polygontubeprimitive3d.obj		\
        $(SLO)$/sdrcubeprimitive3d.obj			\
        $(SLO)$/sdrdecompositiontools3d.obj		\
        $(SLO)$/sdrextrudelathetools3d.obj		\
        $(SLO)$/sdrextrudeprimitive3d.obj		\
        $(SLO)$/sdrlatheprimitive3d.obj			\
        $(SLO)$/sdrpolypolygonprimitive3d.obj	\
        $(SLO)$/sdrprimitive3d.obj				\
        $(SLO)$/sdrsphereprimitive3d.obj		\
        $(SLO)$/shadowprimitive3d.obj			\
        $(SLO)$/textureprimitive3d.obj			\
        $(SLO)$/transformprimitive3d.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
