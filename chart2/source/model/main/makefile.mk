#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 01:08:25 $
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
TARGET=				chmodelmain

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/Axis.obj \
    $(SLO)$/BoundedCoordinateSystem.obj \
    $(SLO)$/CachedDataSequence.obj \
    $(SLO)$/CartesianCoordinateSystem.obj \
    $(SLO)$/ChartModel.obj		\
    $(SLO)$/DataInterpreter.obj \
    $(SLO)$/DataPoint.obj \
    $(SLO)$/DataPointProperties.obj \
    $(SLO)$/DataPointStyle.obj \
    $(SLO)$/DataSeries.obj \
    $(SLO)$/DataSeriesStyle.obj \
    $(SLO)$/Diagram.obj \
    $(SLO)$/DataSeriesProperties.obj \
    $(SLO)$/FileDataProvider.obj \
    $(SLO)$/FileDataSource.obj \
    $(SLO)$/FormattedString.obj \
    $(SLO)$/Grid.obj \
    $(SLO)$/IdentityTransformation.obj \
    $(SLO)$/ImplChartModel.obj \
    $(SLO)$/Increment.obj \
    $(SLO)$/LayoutContainer.obj \
    $(SLO)$/Legend.obj \
    $(SLO)$/PageBackground.obj \
    $(SLO)$/PolarCoordinateSystem.obj \
    $(SLO)$/PolarTransformation.obj \
    $(SLO)$/Scale.obj \
    $(SLO)$/StyleFamilies.obj \
    $(SLO)$/StyleFamily.obj \
    $(SLO)$/SubGrid.obj \
    $(SLO)$/Title.obj \
    $(SLO)$/XYDataInterpreter.obj \
    $(SLO)$/Wall.obj \
    $(SLO)$/_serviceregistration_model.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
