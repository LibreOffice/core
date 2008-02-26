#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 09:48:49 $
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

PRJNAME=sw
TARGET=unoidl
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(DVO_XFORMS)" != ""
CDEFS+=-DDVO_XFORMS
.ENDIF

# --- Files --------------------------------------------------------

SLO1FILES =  \
        $(SLO)$/unodefaults.obj\
        $(SLO)$/unodispatch.obj\
        $(SLO)$/unotxdoc.obj\
        $(SLO)$/unoatxt.obj \
        $(SLO)$/unomailmerge.obj \
        $(SLO)$/unomod.obj \
        $(SLO)$/unotxvw.obj \
        $(SLO)$/dlelstnr.obj \
        $(SLO)$/unofreg.obj \
        $(SLO)$/SwXDocumentSettings.obj \
        $(SLO)$/SwXPrintPreviewSettings.obj \
        $(SLO)$/SwXFilterOptions.obj\
        $(SLO)$/RefreshListenerContainer.obj \
        $(SLO)$/unomodule.obj \
        $(SLO)$/unodoc.obj \
                $(SLO)$/warnpassword.obj

SLO2FILES = \
    $(SLO)$/swdetect.obj		\
    $(SLO)$/swdet2.obj		\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

