#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 01:09:50 $
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

PRJNAME=svx
TARGET=unoedit
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SVXLIGHTOBJFILES=\
        $(OBJ)$/UnoForbiddenCharsTable.obj \
        $(OBJ)$/unoedsrc.obj	\
        $(OBJ)$/unoedhlp.obj	\
        $(OBJ)$/unopracc.obj	\
        $(OBJ)$/unoedprx.obj	\
        $(OBJ)$/unoviwed.obj	\
        $(OBJ)$/unoviwou.obj	\
        $(OBJ)$/unofored.obj	\
        $(OBJ)$/unoforou.obj	\
        $(OBJ)$/unotext.obj		\
        $(OBJ)$/unotext2.obj	\
        $(OBJ)$/unofield.obj

SLOFILES =	\
        $(SLO)$/UnoForbiddenCharsTable.obj \
        $(SLO)$/unoedsrc.obj	\
        $(SLO)$/unoedhlp.obj	\
        $(SLO)$/unopracc.obj	\
        $(SLO)$/unoedprx.obj	\
        $(SLO)$/unoviwed.obj	\
        $(SLO)$/unoviwou.obj	\
        $(SLO)$/unofored.obj	\
        $(SLO)$/unoforou.obj	\
        $(SLO)$/unotext.obj		\
        $(SLO)$/unotext2.obj	\
        $(SLO)$/unofield.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

