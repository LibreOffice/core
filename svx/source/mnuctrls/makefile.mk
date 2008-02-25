#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:19:52 $
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

ENABLE_EXCEPTIONS=TRUE

PRJNAME=svx
TARGET=mnuctrls

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        mnuctrls.src

SLOFILES=	\
        $(SLO)$/clipboardctl.obj \
        $(SLO)$/fntctl.obj \
        $(SLO)$/fntszctl.obj \
        $(SLO)$/SmartTagCtl.obj

HXX1TARGET=mnuctrls
HXX1EXT=   hxx
HXX1EXCL=  -E:*include*
HXX1DEPN=\
        $(INC)$/clipboardctl.hxx \
        $(INC)$/fntctl.hxx	\
        $(INC)$/fntszctl.hxx \
        $(INC)$/SmartTagCtl.hxx

EXCEPTIONSFILES= \
        $(SLO)$/SmartTagCtl.obj


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

