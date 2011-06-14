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

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch

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
        $(SLO)$/algitem.obj		\
        $(SLO)$/chrtitem.obj		\
        $(SLO)$/clipfmtitem.obj		\
        $(SLO)$/customshapeitem.obj	\
        $(SLO)$/drawitem.obj		\
        $(SLO)$/e3ditem.obj		\
        $(SLO)$/grfitem.obj

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
        $(SLO)$/SmartTagItem.obj	\
        $(SLO)$/hlnkitem.obj		\
        $(SLO)$/numfmtsh.obj		\
        $(SLO)$/numinf.obj		\
        $(SLO)$/ofaitem.obj		\
        $(SLO)$/pageitem.obj		\
        $(SLO)$/postattr.obj		\
        $(SLO)$/rotmodit.obj		\
        $(SLO)$/svxempty.obj		\
        $(SLO)$/svxerr.obj		\
        $(SLO)$/viewlayoutitem.obj	\
        $(SLO)$/zoomitem.obj		\
        $(SLO)$/zoomslideritem.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

EXCEPTIONSFILES= \
        $(SLO)$/chrtitem.obj		\
        $(SLO)$/customshapeitem.obj	\
        $(SLO)$/numfmtsh.obj

.INCLUDE :	target.mk

