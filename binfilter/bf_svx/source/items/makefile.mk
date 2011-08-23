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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..
BFPRJ=..$/..

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(BFPRJ)$/util$/svx_svxpch

PRJNAME=binfilter
TARGET=svx_items

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_svx

# --- Files --------------------------------------------------------

SRS1NAME=svx_svxitems
SRC1FILES =  \
        svx_svxerr.src		\
        svx_svxitems.src

SLOFILES=	\
        $(SLO)$/svx_writingmodeitem.obj \
        $(SLO)$/svx_frmitems.obj \
        $(SLO)$/svx_paraitem.obj \
        $(SLO)$/svx_textitem.obj \
        $(SLO)$/svx_postattr.obj	\
        $(SLO)$/svx_flditem.obj \
        $(SLO)$/svx_hlnkitem.obj \
        $(SLO)$/svx_svxfont.obj \
        $(SLO)$/svx_pageitem.obj	\
        $(SLO)$/svx_paperinf.obj	\
        $(SLO)$/svx_drawitem.obj	\
        $(SLO)$/svx_algitem.obj \
        $(SLO)$/svx_rotmodit.obj \
        $(SLO)$/svx_svxerr.obj	\
        $(SLO)$/svx_zoomitem.obj \
        $(SLO)$/svx_chrtitem.obj	\
        $(SLO)$/svx_bulitem.obj \
        $(SLO)$/svx_e3ditem.obj \
        $(SLO)$/svx_numitem.obj \
        $(SLO)$/svx_grfitem.obj \
        $(SLO)$/svx_clipfmtitem.obj \
        $(SLO)$/svx_xmlcnitm.obj

EXCEPTIONSFILES= \
        $(SLO)$/svx_paraitem.obj \
        $(SLO)$/svx_frmitems.obj \
        $(SLO)$/svx_numitem.obj\
        $(SLO)$/svx_xmlcnitm.obj

.INCLUDE :	target.mk

