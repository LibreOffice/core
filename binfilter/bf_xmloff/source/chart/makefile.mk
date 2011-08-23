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

PRJ					= ..$/..$/..
PRJNAME				= binfilter
TARGET				= xmloff_chart
ENABLE_EXCEPTIONS	= TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

INC+= -I$(PRJ)$/inc$/bf_xmloff

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/xmloff_SchXMLExport.obj \
            $(SLO)$/xmloff_SchXMLImport.obj \
            $(SLO)$/xmloff_contexts.obj \
            $(SLO)$/xmloff_SchXMLTableContext.obj \
            $(SLO)$/xmloff_SchXMLChartContext.obj \
            $(SLO)$/xmloff_SchXMLPlotAreaContext.obj \
            $(SLO)$/xmloff_SchXMLParagraphContext.obj \
            $(SLO)$/xmloff_PropertyMaps.obj \
            $(SLO)$/xmloff_XMLChartStyleContext.obj \
            $(SLO)$/xmloff_XMLErrorIndicatorPropertyHdl.obj \
            $(SLO)$/xmloff_SchXMLAutoStylePoolP.obj \
            $(SLO)$/xmloff_XMLChartPropertyContext.obj \
            $(SLO)$/xmloff_XMLSymbolImageContext.obj \
            $(SLO)$/xmloff_XMLTextOrientationHdl.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

