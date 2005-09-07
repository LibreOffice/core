#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 00:36:15 $
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
TARGET=				chcontroller

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES =  \
            $(SLO)$/ConfigurationAccess.obj	\
            $(SLO)$/ViewSingletons.obj	\
            $(SLO)$/ResId.obj \
            $(SLO)$/SelectionHelper.obj	\
            $(SLO)$/PositionAndSizeHelper.obj	\
            $(SLO)$/ChartWindow.obj	\
            $(SLO)$/ChartController.obj	\
            $(SLO)$/ChartController_EditData.obj	\
            $(SLO)$/ChartController_Window.obj	\
            $(SLO)$/ChartController_Properties.obj	\
            $(SLO)$/ChartController_Insert.obj	\
            $(SLO)$/ChartController_TextEdit.obj \
            $(SLO)$/ChartController_Position.obj \
            $(SLO)$/ChartFrameloader.obj \
            $(SLO)$/_serviceregistration_controller.obj

#			$(SLO)$/CommonConverters.obj \
#			$(SLO)$/Scaling.obj	\

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
