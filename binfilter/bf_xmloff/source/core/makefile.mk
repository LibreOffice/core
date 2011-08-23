#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..

PRJNAME=binfilter
TARGET=xmloff_core

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
CDEFS+=-DCONV_STAR_FONTS

INC+= -I$(PRJ)$/inc$/bf_xmloff

# --- to build xmlkywd.obj in obj, too -----------------------------

OBJFILES =  $(OBJ)$/xmloff_xmlkywd.obj
LIB2TARGET =$(LB)$/xmloff_xmlkywd.lib
LIB2OBJFILES  =$(OBJFILES)

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/xmloff_facreg.obj		\
        $(SLO)$/xmloff_xmlcnitm.obj	\
        $(SLO)$/xmloff_attrlist.obj	\
        $(SLO)$/xmloff_i18nmap.obj		\
        $(SLO)$/xmloff_nmspmap.obj		\
        $(SLO)$/xmloff_unoatrcn.obj	\
        $(SLO)$/xmloff_xmlehelp.obj	\
        $(SLO)$/xmloff_xmlerror.obj	\
        $(SLO)$/xmloff_xmlexp.obj		\
        $(SLO)$/xmloff_xmlictxt.obj	\
        $(SLO)$/xmloff_xmlimp.obj		\
        $(SLO)$/xmloff_xmlkywd.obj		\
        $(SLO)$/xmloff_xmltkmap.obj	\
        $(SLO)$/xmloff_xmltoken.obj	\
        $(SLO)$/xmloff_xmluconv.obj	\
        $(SLO)$/xmloff_ProgressBarHelper.obj	\
        $(SLO)$/xmloff_PropertySetMerger.obj	\
        $(SLO)$/xmloff_SettingsExportHelper.obj	\
        $(SLO)$/xmloff_DocumentSettingsContext.obj \
        $(SLO)$/xmloff_XMLEmbeddedObjectExportFilter.obj \
        $(SLO)$/xmloff_XMLEmbeddedObjectImportContext.obj \
        $(SLO)$/xmloff_XMLBase64Export.obj \
        $(SLO)$/xmloff_XMLBase64ImportContext.obj \
        $(SLO)$/xmloff_XMLBasicExportFilter.obj

CPPUMAKERFLAGS=

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
