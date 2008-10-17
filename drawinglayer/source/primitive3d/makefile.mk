#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: aw $ $Date: 2008-06-24 15:31:08 $
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
        $(SLO)$/hittestprimitive3d.obj			\
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
