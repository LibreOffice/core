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
# $Revision: 1.28 $
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

PRJNAME=xmloff
TARGET=core
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

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
        $(SLO)$/XMLBasicExportFilter.obj \
        $(SLO)$/RDFaImportHelper.obj \
        $(SLO)$/RDFaExportHelper.obj \


# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
