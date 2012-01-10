#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ					= ..$/..
PRJNAME				= xmloff
TARGET				= chart
AUTOSEG				= true
ENABLE_EXCEPTIONS	= TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/ColorPropertySet.obj \
            $(SLO)$/SchXMLTools.obj \
            $(SLO)$/SchXMLExport.obj \
            $(SLO)$/SchXMLImport.obj \
            $(SLO)$/contexts.obj \
            $(SLO)$/SchXMLTableContext.obj \
            $(SLO)$/SchXMLChartContext.obj \
            $(SLO)$/SchXMLLegendContext.obj \
            $(SLO)$/SchXMLPlotAreaContext.obj \
            $(SLO)$/SchXMLAxisContext.obj \
            $(SLO)$/SchXMLParagraphContext.obj \
            $(SLO)$/SchXMLTextListContext.obj \
            $(SLO)$/SchXMLSeriesHelper.obj \
            $(SLO)$/SchXMLSeries2Context.obj \
            $(SLO)$/SchXMLEnumConverter.obj \
            $(SLO)$/PropertyMaps.obj \
            $(SLO)$/XMLChartStyleContext.obj \
            $(SLO)$/XMLErrorIndicatorPropertyHdl.obj \
            $(SLO)$/XMLErrorBarStylePropertyHdl.obj \
            $(SLO)$/SchXMLAutoStylePoolP.obj \
            $(SLO)$/XMLChartPropertyContext.obj \
            $(SLO)$/XMLSymbolImageContext.obj \
            $(SLO)$/XMLLabelSeparatorContext.obj \
            $(SLO)$/XMLTextOrientationHdl.obj \
            $(SLO)$/XMLSymbolTypePropertyHdl.obj \
            $(SLO)$/XMLAxisPositionPropertyHdl.obj \
            $(SLO)$/SchXMLCalculationSettingsContext.obj \
            $(SLO)$/transporttypes.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

