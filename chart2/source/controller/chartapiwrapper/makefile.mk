#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: vg $ $Date: 2007-05-22 17:25:28 $
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
TARGET=				chchartapiwrapper

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/Chart2ModelContact.obj \
    $(SLO)$/AreaWrapper.obj \
    $(SLO)$/AxisWrapper.obj \
    $(SLO)$/ChartDataWrapper.obj \
    $(SLO)$/ChartDocumentWrapper.obj \
    $(SLO)$/DataSeriesPointWrapper.obj \
    $(SLO)$/DiagramWrapper.obj \
    $(SLO)$/GridWrapper.obj \
    $(SLO)$/LegendWrapper.obj \
    $(SLO)$/TitleWrapper.obj \
    $(SLO)$/MinMaxLineWrapper.obj \
    $(SLO)$/UpDownBarWrapper.obj \
    $(SLO)$/WallFloorWrapper.obj \
    $(SLO)$/WrappedNamedProperty.obj \
    $(SLO)$/WrappedCharacterHeightProperty.obj \
    $(SLO)$/WrappedDataCaptionProperties.obj \
    $(SLO)$/WrappedTextRotationProperty.obj \
    $(SLO)$/WrappedGapwidthProperty.obj \
    $(SLO)$/WrappedScaleProperty.obj \
    $(SLO)$/WrappedSplineProperties.obj \
    $(SLO)$/WrappedStockProperties.obj \
    $(SLO)$/WrappedSymbolProperties.obj \
    $(SLO)$/WrappedAxisAndGridExistenceProperties.obj \
    $(SLO)$/WrappedNumberFormatProperty.obj \
    $(SLO)$/WrappedStatisticProperties.obj \
    $(SLO)$/WrappedSceneProperty.obj \
    $(SLO)$/WrappedSeriesAreaOrLineProperty.obj \
    $(SLO)$/WrappedAddInProperty.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
