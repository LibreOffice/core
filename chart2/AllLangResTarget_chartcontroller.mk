# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_AllLangResTarget_AllLangResTarget,chartcontroller))

$(eval $(call gb_AllLangResTarget_set_reslocation,chartcontroller,chart2))

$(eval $(call gb_AllLangResTarget_add_srs,chartcontroller,\
    chart2/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,chart2/res))

$(eval $(call gb_SrsTarget_set_include,chart2/res,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/chart2/source/controller/dialogs) \
    -I$(realpath $(SRCDIR)/chart2/source/controller/inc) \
    -I$(realpath $(SRCDIR)/chart2/source/controller/main) \
    -I$(realpath $(SRCDIR)/chart2/source/inc) \
    -I$(WORKDIR)/inc/chart2 \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_SrsTarget_add_files,chart2/res,\
    chart2/source/controller/dialogs/Bitmaps.src \
    chart2/source/controller/dialogs/dlg_ChartType.src \
    chart2/source/controller/dialogs/dlg_CreationWizard.src \
    chart2/source/controller/dialogs/dlg_DataEditor.src \
    chart2/source/controller/dialogs/dlg_DataSource.src \
    chart2/source/controller/dialogs/dlg_InsertAxis_Grid.src \
    chart2/source/controller/dialogs/dlg_InsertDataLabel.src \
    chart2/source/controller/dialogs/dlg_InsertErrorBars.src \
    chart2/source/controller/dialogs/dlg_InsertLegend.src \
    chart2/source/controller/dialogs/dlg_InsertTitle.src \
    chart2/source/controller/dialogs/dlg_InsertTrendline.src \
    chart2/source/controller/dialogs/dlg_ObjectProperties.src \
    chart2/source/controller/dialogs/dlg_ShapeFont.src \
    chart2/source/controller/dialogs/dlg_ShapeParagraph.src \
    chart2/source/controller/dialogs/dlg_View3D.src \
    chart2/source/controller/dialogs/res_BarGeometry.src \
    chart2/source/controller/dialogs/res_TextSeparator.src \
    chart2/source/controller/dialogs/Strings_AdditionalControls.src \
    chart2/source/controller/dialogs/Strings_ChartTypes.src \
    chart2/source/controller/dialogs/Strings_Scale.src \
    chart2/source/controller/dialogs/Strings.src \
    chart2/source/controller/dialogs/Strings_Statistic.src \
    chart2/source/controller/dialogs/tp_3D_SceneAppearance.src \
    chart2/source/controller/dialogs/tp_3D_SceneGeometry.src \
    chart2/source/controller/dialogs/tp_3D_SceneIllumination.src \
    chart2/source/controller/dialogs/tp_AxisLabel.src \
    chart2/source/controller/dialogs/tp_AxisPositions.src \
    chart2/source/controller/dialogs/tp_ChartType.src \
    chart2/source/controller/dialogs/tp_DataLabel.src \
    chart2/source/controller/dialogs/tp_DataSource.src \
    chart2/source/controller/dialogs/tp_ErrorBars.src \
    chart2/source/controller/dialogs/tp_LegendPosition.src \
    chart2/source/controller/dialogs/tp_PointGeometry.src \
    chart2/source/controller/dialogs/tp_PolarOptions.src \
    chart2/source/controller/dialogs/tp_RangeChooser.src \
    chart2/source/controller/dialogs/tp_Scale.src \
    chart2/source/controller/dialogs/tp_SeriesToAxis.src \
    chart2/source/controller/dialogs/tp_TitleRotation.src \
    chart2/source/controller/dialogs/tp_Trendline.src \
    chart2/source/controller/dialogs/tp_Wizard_TitlesAndObjects.src \
    chart2/source/controller/menus/ShapeContextMenu.src \
    chart2/source/controller/menus/ShapeEditContextMenu.src \
))

$(eval $(call gb_SrsTarget_add_templates,chart2/res,\
    chart2/source/controller/dialogs/res_DataLabel_tmpl.hrc \
    chart2/source/controller/dialogs/res_ErrorBar_tmpl.hrc \
    chart2/source/controller/dialogs/res_LegendPosition_tmpl.hrc \
    chart2/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc \
    chart2/source/controller/dialogs/res_Titlesx_tmpl.hrc \
    chart2/source/controller/dialogs/res_Trendline_tmpl.hrc \
))

# vim: set noet ts=4 sw=4:
