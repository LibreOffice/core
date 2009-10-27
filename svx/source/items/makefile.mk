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
# $Revision: 1.19 $
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

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch
#ENABLE_EXCEPTIONS=TRUE

PRJNAME=svx
TARGET=items
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=svxitems
SRC1FILES =  \
        svxerr.src		\
        svxitems.src

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
        $(SLO)$/writingmodeitem.obj \
        $(SLO)$/frmitems.obj \
        $(SLO)$/paraitem.obj \
        $(SLO)$/textitem.obj \
        $(SLO)$/flditem.obj \
        $(SLO)$/svxfont.obj \
        $(SLO)$/paperinf.obj    \
        $(SLO)$/drawitem.obj	\
        $(SLO)$/itemtype.obj	\
        $(SLO)$/chrtitem.obj	\
        $(SLO)$/bulitem.obj \
        $(SLO)$/e3ditem.obj \
        $(SLO)$/numitem.obj \
        $(SLO)$/grfitem.obj \
        $(SLO)$/clipfmtitem.obj \
        $(SLO)$/xmlcnitm.obj \
        $(SLO)$/customshapeitem.obj \
        $(SLO)$/charhiddenitem.obj

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
        $(SLO)$/ofaitem.obj \
        $(SLO)$/postattr.obj	\
        $(SLO)$/hlnkitem.obj \
        $(SLO)$/pageitem.obj	\
                $(SLO)$/viewlayoutitem.obj    \
        $(SLO)$/algitem.obj \
        $(SLO)$/rotmodit.obj \
        $(SLO)$/numinf.obj	\
        $(SLO)$/svxerr.obj	\
        $(SLO)$/numfmtsh.obj	\
        $(SLO)$/zoomitem.obj \
        $(SLO)$/svxempty.obj \
        $(SLO)$/SmartTagItem.obj \
        $(SLO)$/zoomslideritem.obj \

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

EXCEPTIONSFILES= \
        $(SLO)$/paraitem.obj \
        $(SLO)$/frmitems.obj \
        $(SLO)$/numitem.obj\
        $(SLO)$/xmlcnitm.obj\
        $(SLO)$/flditem.obj \
        $(SLO)$/customshapeitem.obj

.INCLUDE :	target.mk

