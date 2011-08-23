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
PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PRJNAME					= binfilter
PROJECTPCH				= sch
PROJECTPCHSOURCE		= $(BFPRJ)$/util$/sch_sch
TARGET					= sch_chxchart
ENABLE_EXCEPTIONS		= TRUE
BOOTSTRAP_SERVICE		= FALSE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sch

.IF "$(GUI)" == "WNT"
CFLAGS+=-Zm500
.ENDIF

# --- Files --------------------------------------------------------

CXXFILES = \
    sch_ChXChartAxis.cxx									\
    sch_ChXChartData.cxx									\
    sch_ChXChartDataChangeEventListener.cxx					\
    sch_ChXChartDocument.cxx								\
    sch_ChXChartObject.cxx									\
    sch_ChXDataPoint.cxx									\
    sch_ChXDataRow.cxx										\
    sch_ChXDiagram.cxx										\
    sch_mapprov.cxx											\
    sch_ChXChartDrawPage.cxx								\
    sch_ChartLegend.cxx										\
    sch_ChartTitle.cxx										\
    sch_ChartLegend.cxx										\
    sch_ChartArea.cxx										\
    sch_ChartLine.cxx										\
    sch_ChartGrid.cxx										\
    sch_register.cxx										\
    sch_unodoc.cxx

SLOFILES =  \
    $(SLO)$/sch_ChXChartAxis.obj							\
    $(SLO)$/sch_ChXChartData.obj							\
    $(SLO)$/sch_ChXChartDataChangeEventListener.obj			\
    $(SLO)$/sch_ChXChartDocument.obj						\
    $(SLO)$/sch_ChXChartObject.obj							\
    $(SLO)$/sch_ChXDataPoint.obj							\
    $(SLO)$/sch_ChXDataRow.obj								\
    $(SLO)$/sch_ChXDiagram.obj								\
    $(SLO)$/sch_mapprov.obj									\
    $(SLO)$/sch_ChXChartDrawPage.obj						\
    $(SLO)$/sch_ChartLegend.obj								\
    $(SLO)$/sch_ChartTitle.obj								\
    $(SLO)$/sch_ChartArea.obj								\
    $(SLO)$/sch_ChartLine.obj								\
    $(SLO)$/sch_ChartGrid.obj								\
    $(SLO)$/sch_register.obj								\
    $(SLO)$/sch_unodoc.obj

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
