#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: aw $ $Date: 2008-04-04 06:00:23 $
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
TARGET=primitive2d
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SLOFILES= \
        $(SLO)$/alphaprimitive2d.obj			\
        $(SLO)$/animatedprimitive2d.obj			\
        $(SLO)$/baseprimitive2d.obj				\
        $(SLO)$/backgroundcolorprimitive2d.obj	\
        $(SLO)$/bitmapprimitive2d.obj			\
        $(SLO)$/borderlineprimitive2d.obj		\
        $(SLO)$/chartprimitive2d.obj			\
        $(SLO)$/controlprimitive2d.obj			\
        $(SLO)$/embedded3dprimitive2d.obj		\
        $(SLO)$/fillbitmapprimitive2d.obj		\
        $(SLO)$/fillgradientprimitive2d.obj		\
        $(SLO)$/fillhatchprimitive2d.obj		\
        $(SLO)$/graphicprimitive2d.obj			\
        $(SLO)$/gridprimitive2d.obj				\
        $(SLO)$/groupprimitive2d.obj			\
        $(SLO)$/helplineprimitive2d.obj			\
        $(SLO)$/markerarrayprimitive2d.obj		\
        $(SLO)$/pointarrayprimitive2d.obj		\
        $(SLO)$/maskprimitive2d.obj				\
        $(SLO)$/mediaprimitive2d.obj			\
        $(SLO)$/metafileprimitive2d.obj			\
        $(SLO)$/modifiedcolorprimitive2d.obj	\
        $(SLO)$/pagepreviewprimitive2d.obj		\
        $(SLO)$/polypolygonprimitive2d.obj		\
        $(SLO)$/polygonprimitive2d.obj			\
        $(SLO)$/sceneprimitive2d.obj			\
        $(SLO)$/shadowprimitive2d.obj			\
        $(SLO)$/structuretagprimitive2d.obj		\
        $(SLO)$/texteffectprimitive2d.obj		\
        $(SLO)$/textlayoutdevice.obj			\
        $(SLO)$/textprimitive2d.obj				\
        $(SLO)$/textdecoratedprimitive2d.obj	\
        $(SLO)$/texthierarchyprimitive2d.obj	\
        $(SLO)$/transformprimitive2d.obj		\
        $(SLO)$/unifiedalphaprimitive2d.obj		\
        $(SLO)$/wrongspellprimitive2d.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
