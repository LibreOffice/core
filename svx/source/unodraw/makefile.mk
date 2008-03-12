#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 10:09:56 $
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
TARGET=unodraw
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/UnoNamespaceMap.obj \
        $(SLO)$/UnoGraphicExporter.obj \
        $(SLO)$/unopool.obj \
        $(SLO)$/XPropertyTable.obj \
        $(SLO)$/UnoNameItemTable.obj \
        $(SLO)$/unoshape.obj	\
        $(SLO)$/unoipset.obj	\
        $(SLO)$/unoctabl.obj	\
        $(SLO)$/unoshap2.obj	\
        $(SLO)$/unoshap3.obj	\
        $(SLO)$/unoshap4.obj	\
        $(SLO)$/unopage.obj		\
        $(SLO)$/unoshtxt.obj	\
        $(SLO)$/unoshcol.obj	\
        $(SLO)$/unoprov.obj		\
        $(SLO)$/unomod.obj      \
        $(SLO)$/unonrule.obj	\
        $(SLO)$/unofdesc.obj	\
        $(SLO)$/unomlstr.obj	\
        $(SLO)$/unogtabl.obj	\
        $(SLO)$/unohtabl.obj	\
        $(SLO)$/unobtabl.obj	\
        $(SLO)$/unottabl.obj	\
        $(SLO)$/unomtabl.obj	\
        $(SLO)$/unodtabl.obj	\
        $(SLO)$/gluepts.obj     \
        $(SLO)$/recoveryui.obj	\
        $(SLO)$/tableshape.obj

SRS1NAME=unodraw
SRC1FILES =  \
        unodraw.src

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

