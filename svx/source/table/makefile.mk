#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 10:01:40 $
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

