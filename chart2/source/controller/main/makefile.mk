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
