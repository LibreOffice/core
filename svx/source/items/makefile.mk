#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: avy $ $Date: 2001-02-28 12:00:29 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch
#ENABLE_EXCEPTIONS=TRUE

PRJNAME=svx
TARGET=items
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :	svpre.mk
.INCLUDE :	settings.mk
.INCLUDE :	sv.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=svxitems
SRC1FILES =  \
        svxerr.src		\
        svxitems.src

SLOFILES=	\
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
        $(SLO)$/svxempty.obj

EXCEPTIONSFILES= \
        $(SLO)$/paraitem.obj \
        $(OBJ)$/sxl_paraitem.obj\
        $(SLO)$/numitem.obj\
                $(OBJ)$/sxl_numitem.obj

SVXLIGHTOBJFILES=	\
        $(OBJ)$/frmitems.obj \
        $(OBJ)$/paraitem.obj \
        $(OBJ)$/textitem.obj \
        $(OBJ)$/postattr.obj	\
        $(OBJ)$/flditem.obj \
        $(OBJ)$/svxfont.obj \
        $(OBJ)$/pageitem.obj	\
        $(OBJ)$/paperinf.obj	\
        $(OBJ)$/drawitem.obj	\
        $(OBJ)$/algitem.obj \
        $(OBJ)$/itemtype.obj	\
        $(OBJ)$/rotmodit.obj \
        $(OBJ)$/numinf.obj	\
        $(OBJ)$/svxerr.obj	\
        $(OBJ)$/numfmtsh.obj	\
        $(OBJ)$/zoomitem.obj \
        $(OBJ)$/chrtitem.obj	\
        $(OBJ)$/bulitem.obj \
        $(OBJ)$/e3ditem.obj \
        $(OBJ)$/numitem.obj \
        $(OBJ)$/grfitem.obj \

.INCLUDE :	target.mk

