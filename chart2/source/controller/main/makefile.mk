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

PRJ=				..$/..$/..
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chcontroller

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES =  \
            $(SLO)$/ConfigurationAccess.obj	\
            $(SLO)$/ElementSelector.obj	\
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
            $(SLO)$/ChartController_Tools.obj \
            $(SLO)$/ChartFrameloader.obj \
            $(SLO)$/ChartRenderer.obj \
            $(SLO)$/CommandDispatchContainer.obj \
            $(SLO)$/CommandDispatch.obj \
            $(SLO)$/ControllerCommandDispatch.obj \
            $(SLO)$/UndoCommandDispatch.obj \
            $(SLO)$/DragMethod_Base.obj \
            $(SLO)$/DragMethod_RotateDiagram.obj \
            $(SLO)$/DragMethod_PieSegment.obj \
            $(SLO)$/ObjectHierarchy.obj \
            $(SLO)$/_serviceregistration_controller.obj \
            $(SLO)$/ChartDropTargetHelper.obj \
            $(SLO)$/StatusBarCommandDispatch.obj \
            $(SLO)$/ChartTransferable.obj \
            $(SLO)$/FeatureCommandDispatchBase.obj \
            $(SLO)$/DrawCommandDispatch.obj \
            $(SLO)$/ShapeController.obj \
            $(SLO)$/ShapeToolbarController.obj \
            $(SLO)$/UndoActions.obj \
            $(SLO)$/UndoGuard.obj \
            $(SLO)$/ChartModelClone.obj \

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
