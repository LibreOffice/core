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
# $Revision: 1.10 $
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

PRJ=				..$/..$/..
PRJNAME=			chart2
TARGET=				chmodeltemplate

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# i26518 the gcc-3.0.4 requires to enhance the template-depth
# this seems to be a compiler issue, so we recommend not to use 3.0.x anymore
.IF "$(COM)"=="GCC" 
    CFLAGS+=-ftemplate-depth-128
.ENDIF

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/AreaChartType.obj \
    $(SLO)$/AreaChartTypeTemplate.obj \
    $(SLO)$/BarChartType.obj \
    $(SLO)$/BarChartTypeTemplate.obj \
    $(SLO)$/CandleStickChartType.obj \
    $(SLO)$/ChartType.obj		\
    $(SLO)$/ChartTypeManager.obj		\
    $(SLO)$/ChartTypeTemplate.obj		\
    $(SLO)$/ColumnChartType.obj \
    $(SLO)$/ColumnLineChartTypeTemplate.obj \
    $(SLO)$/DataInterpreter.obj \
    $(SLO)$/LineChartType.obj \
    $(SLO)$/LineChartTypeTemplate.obj \
    $(SLO)$/NetChartType.obj \
    $(SLO)$/NetChartTypeTemplate.obj \
    $(SLO)$/PieChartType.obj \
    $(SLO)$/PieChartTypeTemplate.obj \
    $(SLO)$/ScatterChartType.obj \
    $(SLO)$/ScatterChartTypeTemplate.obj \
    $(SLO)$/BubbleChartType.obj \
    $(SLO)$/BubbleChartTypeTemplate.obj \
    $(SLO)$/BubbleDataInterpreter.obj \
    $(SLO)$/StockChartTypeTemplate.obj \
    $(SLO)$/StockDataInterpreter.obj \
    $(SLO)$/XYDataInterpreter.obj \
    $(SLO)$/ColumnLineDataInterpreter.obj \
    $(SLO)$/_serviceregistration_charttypes.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
