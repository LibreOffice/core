#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2006-10-12 12:54:39 $
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
#ENABLE_EXCEPTIONS=TRUE

PRJNAME=svx
TARGET=items

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=svxitems
SRC1FILES =  \
        svxerr.src		\
        svxitems.src

SLOFILES=	\
        $(SLO)$/ofaitem.obj \
        $(SLO)$/writingmodeitem.obj \
        $(SLO)$/frmitems.obj \
        $(SLO)$/paraitem.obj \
        $(SLO)$/textitem.obj \
        $(SLO)$/postattr.obj	\
        $(SLO)$/flditem.obj \
        $(SLO)$/hlnkitem.obj \
        $(SLO)$/svxfont.obj \
        $(SLO)$/pageitem.obj	\
        $(SLO)$/paperinf.obj	\
        $(SLO)$/drawitem.obj	\
        $(SLO)$/algitem.obj \
        $(SLO)$/itemtype.obj	\
        $(SLO)$/rotmodit.obj \
        $(SLO)$/numinf.obj	\
        $(SLO)$/svxerr.obj	\
        $(SLO)$/numfmtsh.obj	\
        $(SLO)$/zoomitem.obj \
        $(SLO)$/chrtitem.obj	\
        $(SLO)$/bulitem.obj \
        $(SLO)$/e3ditem.obj \
        $(SLO)$/numitem.obj \
        $(SLO)$/grfitem.obj \
        $(SLO)$/clipfmtitem.obj \
        $(SLO)$/svxempty.obj \
        $(SLO)$/xmlcnitm.obj \
        $(SLO)$/customshapeitem.obj \
        $(SLO)$/charhiddenitem.obj

EXCEPTIONSFILES= \
        $(SLO)$/paraitem.obj \
        $(SLO)$/frmitems.obj \
        $(SLO)$/numitem.obj\
        $(SLO)$/xmlcnitm.obj\
        $(SLO)$/flditem.obj \
        $(SLO)$/customshapeitem.obj

.INCLUDE :	target.mk

