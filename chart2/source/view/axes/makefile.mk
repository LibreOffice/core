#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: vg $ $Date: 2007-05-22 19:14:10 $
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
TARGET=				chvaxes

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
#.IF "$(GUI)" == "WNT"
#CFLAGS+=-GR
#.ENDIF

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES = \
    $(SLO)$/VAxisOrGridBase.obj \
    $(SLO)$/VAxisBase.obj \
    $(SLO)$/TickmarkHelper.obj \
    $(SLO)$/MinimumAndMaximumSupplier.obj \
    $(SLO)$/ScaleAutomatism.obj \
    $(SLO)$/VAxisProperties.obj \
    $(SLO)$/VCartesianAxis.obj \
    $(SLO)$/VCartesianGrid.obj \
    $(SLO)$/VCartesianCoordinateSystem.obj \
    $(SLO)$/VPolarAxis.obj \
    $(SLO)$/VPolarAngleAxis.obj \
    $(SLO)$/VPolarRadiusAxis.obj \
    $(SLO)$/VPolarGrid.obj \
    $(SLO)$/VPolarCoordinateSystem.obj \
    $(SLO)$/ExplicitCategoriesProvider.obj \
    $(SLO)$/VCoordinateSystem.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
