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

#PROJECTPCH4DLL=TRUE
#PROJECTPCH=svxpch
#PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch
#ENABLE_EXCEPTIONS=TRUE

PRJNAME=editeng
TARGET=items

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=items
SRC1FILES =  \
        page.src \
        svxitems.src

SLOFILES= \
	$(EXCEPTIONSFILES) \
        $(SLO)$/charhiddenitem.obj \
        $(SLO)$/justifyitem.obj \
        $(SLO)$/optitems.obj \
        $(SLO)$/writingmodeitem.obj

EXCEPTIONSFILES= \
        $(SLO)$/bulitem.obj \
        $(SLO)$/flditem.obj \
        $(SLO)$/frmitems.obj \
        $(SLO)$/itemtype.obj \
        $(SLO)$/numitem.obj \
        $(SLO)$/paperinf.obj \
        $(SLO)$/paraitem.obj \
        $(SLO)$/svdfield.obj \
        $(SLO)$/svxfont.obj \
        $(SLO)$/textitem.obj \
        $(SLO)$/xmlcnitm.obj

.INCLUDE :	target.mk

