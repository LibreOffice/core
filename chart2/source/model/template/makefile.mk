#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 01:25:18 $
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

PRJ=				..$/..$/..
PRJNAME=			chart2
TARGET=				chmodeltemplate

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE

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
    $(SLO)$/ChartType.obj		\
    $(SLO)$/ChartTypeManager.obj		\
    $(SLO)$/ChartTypeTemplate.obj		\
    $(SLO)$/ColumnChartType.obj \
    $(SLO)$/ColumnLineChartTypeTemplate.obj \
    $(SLO)$/DoubleStackedBarChartTypeTemplate.obj \
    $(SLO)$/LineChartType.obj \
    $(SLO)$/LineChartTypeTemplate.obj \
    $(SLO)$/NetChartType.obj \
    $(SLO)$/NetChartTypeTemplate.obj \
    $(SLO)$/PieChartType.obj \
    $(SLO)$/PieChartTypeTemplate.obj \
    $(SLO)$/ScatterChartType.obj \
    $(SLO)$/ScatterChartTypeTemplate.obj \
    $(SLO)$/StockChartTypeTemplate.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
