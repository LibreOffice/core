#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 16:52:56 $
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
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chcitemsetwrapper

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES=   	$(SLO)$/ItemConverter.obj \
                $(SLO)$/AxisItemConverter.obj \
                $(SLO)$/MultipleItemConverter.obj \
                $(SLO)$/MultipleChartConverters.obj \
                $(SLO)$/CharacterPropertyItemConverter.obj \
                $(SLO)$/SeriesOptionsItemConverter.obj \
                $(SLO)$/DataPointItemConverter.obj \
                $(SLO)$/GraphicPropertyItemConverter.obj \
                $(SLO)$/LegendItemConverter.obj \
                $(SLO)$/StatisticsItemConverter.obj \
                $(SLO)$/TitleItemConverter.obj \
                $(SLO)$/RegressionCurveItemConverter.obj \
                $(SLO)$/RegressionEquationItemConverter.obj \
                $(SLO)$/ErrorBarItemConverter.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

