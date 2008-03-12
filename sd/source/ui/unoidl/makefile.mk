#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 11:50:02 $
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


PRJ=..$/..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=unoidl
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true
LIBTARGET = NO
PRJINC=..$/slidesorter

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/unowcntr.obj
.ENDIF

SLO1FILES =      \
        $(SLO)$/DrawController.obj \
        $(SLO)$/SdUnoPresView.obj\
        $(SLO)$/SdUnoSlideView.obj\
        $(SLO)$/SdUnoOutlineView.obj\
        $(SLO)$/SdUnoDrawView.obj \
        $(SLO)$/unopool.obj \
        $(SLO)$/UnoDocumentSettings.obj \
        $(SLO)$/facreg.obj \
        $(SLO)$/unomodel.obj    \
        $(SLO)$/unopage.obj     \
        $(SLO)$/unolayer.obj    \
        $(SLO)$/unopres.obj     \
        $(SLO)$/unocpres.obj    \
        $(SLO)$/unoobj.obj		\
        $(SLO)$/unosrch.obj		\
        $(SLO)$/unowcntr.obj	\
        $(SLO)$/unokywds.obj	\
        $(SLO)$/unopback.obj	\
        $(SLO)$/unovwcrs.obj	\
        $(SLO)$/unodoc.obj      \
        $(SLO)$/unomodule.obj	\
        $(SLO)$/randomnode.obj

SLO2FILES = \
    $(SLO)$/sddetect.obj		\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

