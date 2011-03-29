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
TARGET=primitive2d
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/transparenceprimitive2d.obj			\
        $(SLO)$/animatedprimitive2d.obj				\
        $(SLO)$/baseprimitive2d.obj					\
        $(SLO)$/backgroundcolorprimitive2d.obj		\
        $(SLO)$/bitmapprimitive2d.obj				\
        $(SLO)$/rendergraphicprimitive2d.obj			\
        $(SLO)$/borderlineprimitive2d.obj			\
        $(SLO)$/chartprimitive2d.obj				\
        $(SLO)$/controlprimitive2d.obj				\
        $(SLO)$/discretebitmapprimitive2d.obj		\
        $(SLO)$/discreteshadowprimitive2d.obj		\
        $(SLO)$/embedded3dprimitive2d.obj			\
        $(SLO)$/epsprimitive2d.obj					\
        $(SLO)$/fillbitmapprimitive2d.obj			\
        $(SLO)$/fillgradientprimitive2d.obj			\
        $(SLO)$/fillhatchprimitive2d.obj			\
        $(SLO)$/graphicprimitive2d.obj				\
        $(SLO)$/gridprimitive2d.obj					\
        $(SLO)$/groupprimitive2d.obj				\
        $(SLO)$/helplineprimitive2d.obj				\
        $(SLO)$/hiddengeometryprimitive2d.obj		\
        $(SLO)$/invertprimitive2d.obj				\
        $(SLO)$/markerarrayprimitive2d.obj			\
        $(SLO)$/pointarrayprimitive2d.obj			\
        $(SLO)$/maskprimitive2d.obj					\
        $(SLO)$/mediaprimitive2d.obj				\
        $(SLO)$/metafileprimitive2d.obj				\
        $(SLO)$/modifiedcolorprimitive2d.obj		\
        $(SLO)$/pagepreviewprimitive2d.obj			\
        $(SLO)$/polypolygonprimitive2d.obj			\
        $(SLO)$/polygonprimitive2d.obj				\
        $(SLO)$/primitivetools2d.obj				\
        $(SLO)$/sceneprimitive2d.obj				\
        $(SLO)$/sdrdecompositiontools2d.obj			\
        $(SLO)$/shadowprimitive2d.obj				\
        $(SLO)$/structuretagprimitive2d.obj			\
        $(SLO)$/texteffectprimitive2d.obj			\
        $(SLO)$/textenumsprimitive2d.obj			\
        $(SLO)$/textlayoutdevice.obj				\
        $(SLO)$/textlineprimitive2d.obj				\
        $(SLO)$/textprimitive2d.obj					\
        $(SLO)$/textstrikeoutprimitive2d.obj		\
        $(SLO)$/textdecoratedprimitive2d.obj		\
        $(SLO)$/texthierarchyprimitive2d.obj		\
        $(SLO)$/transformprimitive2d.obj			\
        $(SLO)$/unifiedtransparenceprimitive2d.obj	\
        $(SLO)$/wallpaperprimitive2d.obj			\
        $(SLO)$/wrongspellprimitive2d.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
