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
TARGET=				chcdialogs

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# i26518 the gcc-3.0.4 requires to enhance the template-depth
# this seems to be a compiler issue, so we recommend not to use 3.0.x anymore
.IF "$(COM)"=="GCC" 
    CFLAGS+=-ftemplate-depth-128
.ENDIF

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES=   	\
                $(SLO)$/ObjectNameProvider.obj \
                $(SLO)$/TimerTriggeredControllerLock.obj \
                $(SLO)$/ChangingResource.obj \
                $(SLO)$/DataBrowser.obj \
                $(SLO)$/DataBrowserModel.obj \
                $(SLO)$/DialogModel.obj \
                $(SLO)$/TitleDialogData.obj \
                $(SLO)$/res_Titles.obj \
                $(SLO)$/res_TextSeparator.obj \
                $(SLO)$/res_ErrorBar.obj \
                $(SLO)$/res_Trendline.obj \
                $(SLO)$/res_BarGeometry.obj \
                $(SLO)$/res_LegendPosition.obj \
                $(SLO)$/res_DataLabel.obj \
                $(SLO)$/ChartTypeDialogController.obj \
                $(SLO)$/tp_ChartType.obj \
                $(SLO)$/tp_RangeChooser.obj \
                $(SLO)$/tp_Wizard_TitlesAndObjects.obj \
                $(SLO)$/tp_Location.obj \
                $(SLO)$/tp_AxisLabel.obj \
                $(SLO)$/tp_AxisPositions.obj \
                $(SLO)$/tp_DataLabel.obj \
                $(SLO)$/tp_DataSource.obj \
                $(SLO)$/tp_DataSourceControls.obj \
                $(SLO)$/tp_LegendPosition.obj \
                $(SLO)$/tp_PointGeometry.obj \
                $(SLO)$/tp_Scale.obj \
                $(SLO)$/tp_Trendline.obj \
                $(SLO)$/tp_ErrorBars.obj \
                $(SLO)$/tp_SeriesToAxis.obj \
                $(SLO)$/tp_TitleRotation.obj \
                $(SLO)$/tp_3D_SceneGeometry.obj \
                $(SLO)$/tp_3D_SceneAppearance.obj \
                $(SLO)$/tp_3D_SceneIllumination.obj \
                $(SLO)$/tp_PolarOptions.obj \
                $(SLO)$/dlg_ObjectProperties.obj \
                $(SLO)$/dlg_DataEditor.obj \
                $(SLO)$/dlg_DataSource.obj \
                $(SLO)$/dlg_View3D.obj \
                $(SLO)$/dlg_InsertAxis_Grid.obj \
                $(SLO)$/dlg_InsertDataLabel.obj \
                $(SLO)$/dlg_InsertLegend.obj \
                $(SLO)$/dlg_InsertTrendline.obj \
                $(SLO)$/dlg_InsertErrorBars.obj \
                $(SLO)$/dlg_InsertTitle.obj \
                $(SLO)$/dlg_CreationWizard.obj \
                $(SLO)$/dlg_CreationWizard_UNO.obj \
                $(SLO)$/dlg_ChartType_UNO.obj \
                $(SLO)$/dlg_ChartType.obj \
                $(SLO)$/dlg_NumberFormat.obj \
                $(SLO)$/dlg_ShapeFont.obj \
                $(SLO)$/dlg_ShapeParagraph.obj \
                $(SLO)$/RangeSelectionButton.obj \
                $(SLO)$/RangeSelectionHelper.obj \
                $(SLO)$/RangeSelectionListener.obj \
                $(SLO)$/RangeEdit.obj \
                $(SLO)$/TextDirectionListBox.obj

# --- Resources ---------------------------------------------------------------

SRC1FILES=		\
                res_TextSeparator.src \
                res_BarGeometry.src \
                tp_ChartType.src \
                tp_RangeChooser.src \
                tp_Wizard_TitlesAndObjects.src \
                tp_Location.src \
                tp_AxisLabel.src \
                tp_AxisPositions.src \
                tp_DataLabel.src \
                tp_DataSource.src \
                tp_LegendPosition.src \
                tp_PointGeometry.src \
                tp_Scale.src \
                tp_Trendline.src \
                tp_ErrorBars.src \
                tp_SeriesToAxis.src \
                tp_TitleRotation.src \
                tp_3D_SceneGeometry.src \
                tp_3D_SceneAppearance.src \
                tp_3D_SceneIllumination.src \
                tp_PolarOptions.src \
                dlg_ObjectProperties.src \
                Strings_Statistic.src \
                Strings_Scale.src \
                Strings_ChartTypes.src \
                Strings_AdditionalControls.src \
                Strings.src \
                Bitmaps.src \
                dlg_DataEditor.src \
                dlg_DataSource.src \
                dlg_View3D.src \
                dlg_InsertAxis_Grid.src \
                dlg_InsertDataLabel.src \
                dlg_InsertLegend.src \
                dlg_InsertTrendline.src \
                dlg_InsertErrorBars.src \
                dlg_InsertTitle.src \
                dlg_ChartType.src \
                dlg_CreationWizard.src \
                dlg_ShapeFont.src \
                dlg_ShapeParagraph.src

SRS1NAME=$(TARGET)

# --- Targets -----------------------------------------------------------------
LOCALIZE_ME=res_DataLabel_tmpl.hrc res_LegendPosition_tmpl.hrc res_Trendline_tmpl.hrc res_ErrorBar_tmpl.hrc res_Titlesx_tmpl.hrc res_SecondaryAxisCheckBoxes_tmpl.hrc 
.INCLUDE: target.mk
$(SRS)$/chcdialogs.srs: $(INCCOM)$/res_DataLabel.hrc
$(SRS)$/chcdialogs.srs: $(INCCOM)$/res_LegendPosition.hrc
$(SRS)$/chcdialogs.srs: $(INCCOM)$/res_Trendline.hrc
$(SRS)$/chcdialogs.srs: $(INCCOM)$/res_ErrorBar.hrc
$(SRS)$/chcdialogs.srs: $(INCCOM)$/res_Titlesx.hrc
$(SRS)$/chcdialogs.srs: $(INCCOM)$/res_SecondaryAxisCheckBoxes.hrc

#dependencies:

$(SRS)$/chcdialogs.srs: \
        Bitmaps.hrc \
        CommonResources.hrc \
        ResourceIds.hrc \
        ..$/..$/inc$/Strings.hrc \
        TabPages.hrc \
        dlg_CreationWizard.hrc \
        dlg_DataSource.hrc \
        dlg_InsertAxis_Grid.hrc \
        dlg_InsertDataLabel.hrc \
        dlg_InsertLegend.hrc \
        dlg_InsertTrendline.hrc \
        dlg_InsertErrorBars.hrc \
        dlg_InsertTitle.hrc \
        dlg_View3D.hrc \
        res_Titles.hrc \
        tp_3D_SceneAppearance.hrc \
        tp_3D_SceneGeometry.hrc \
        tp_3D_SceneIllumination.hrc \
        tp_PolarOptions.hrc \
        tp_ChartType.hrc \
        tp_DataSource.hrc \
        tp_Location.hrc \
        tp_RangeChooser.hrc \
        tp_Wizard_TitlesAndObjects.hrc

#        res_DataLabel.hrc \
#        res_LegendPosition.hrc \
#        res_Statistic.hrc \

#$(SRS)$/chcdialogs.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

