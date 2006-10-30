#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.27 $
#
#   last change: $Author: rt $ $Date: 2006-10-30 09:05:42 $
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

PRJNAME=xmloff
TARGET=core
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- to build xmlkywd.obj in obj, too -----------------------------

OBJFILES =  $(OBJ)$/xmlkywd.obj
LIB2TARGET =$(LB)$/xmlkywd.lib
LIB2OBJFILES  =$(OBJFILES)

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/unointerfacetouniqueidentifiermapper.obj \
        $(SLO)$/facreg.obj		\
        $(SLO)$/xmlcnitm.obj	\
        $(SLO)$/attrlist.obj	\
        $(SLO)$/i18nmap.obj		\
        $(SLO)$/nmspmap.obj		\
        $(SLO)$/unoatrcn.obj	\
        $(SLO)$/xmlehelp.obj	\
        $(SLO)$/xmlerror.obj	\
        $(SLO)$/xmlexp.obj		\
        $(SLO)$/xmlictxt.obj	\
        $(SLO)$/xmlimp.obj		\
        $(SLO)$/xmlkywd.obj		\
        $(SLO)$/xmltkmap.obj	\
        $(SLO)$/xmltoken.obj	\
        $(SLO)$/xmluconv.obj	\
        $(SLO)$/DomBuilderContext.obj \
        $(SLO)$/DomExport.obj \
        $(SLO)$/ProgressBarHelper.obj	\
        $(SLO)$/PropertySetMerger.obj	\
        $(SLO)$/SettingsExportHelper.obj	\
        $(SLO)$/DocumentSettingsContext.obj \
        $(SLO)$/XMLEmbeddedObjectExportFilter.obj \
        $(SLO)$/XMLEmbeddedObjectImportContext.obj \
        $(SLO)$/XMLBase64Export.obj \
        $(SLO)$/XMLBase64ImportContext.obj \
        $(SLO)$/XMLBasicExportFilter.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
