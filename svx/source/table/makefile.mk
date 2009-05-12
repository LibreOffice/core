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
# $Revision: 1.3 $
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
TARGET=table
ENABLE_EXCEPTIONS=TRUE;

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=	\
        $(SLO)$/propertyset.obj\
        $(SLO)$/celleditsource.obj\
        $(SLO)$/cell.obj\
        $(SLO)$/cellrange.obj\
        $(SLO)$/cellcursor.obj\
        $(SLO)$/tablerow.obj\
        $(SLO)$/tablerows.obj\
        $(SLO)$/tablecolumn.obj\
        $(SLO)$/tablecolumns.obj\
        $(SLO)$/tablemodel.obj\
        $(SLO)$/svdotable.obj\
        $(SLO)$/viewcontactoftableobj.obj\
        $(SLO)$/tablelayouter.obj\
        $(SLO)$/tablehandles.obj\
        $(SLO)$/tablecontroller.obj\
        $(SLO)$/tabledesign.obj\
        $(SLO)$/tableundo.obj\
        $(SLO)$/accessibletableshape.obj\
        $(SLO)$/accessiblecell.obj \
        $(SLO)$/tablertfexporter.obj \
        $(SLO)$/tablertfimporter.obj

SRS1NAME=table
SRC1FILES= table.src

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

