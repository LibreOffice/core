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
    -I$(WORKDIR)/chart2/inc \
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

chart2_HrcPartTarget__get_target = $(subst _tmpl,,$(notdir $(1)))

define chart2_HrcPartTarget_get_target
$(call chart2_HrcTarget__get_inc_dir,$(1))/$(call chart2_HrcPartTarget__get_target,$(1))
endef

define chart2_HrcPartTarget_HrcPartTarget
ifeq ($(strip $(WITH_LANG)),)
$(call chart2_HrcPartTarget_get_target,$(1)) : $(SRCDIR)/$(1)
	mkdir -p $$(dir $$@) && cp $$< $$@
else
$(call chart2_HrcPartTarget_get_target,$(1)) :| $(gb_SrsPartMergeTarget_TRANSEXTARGET)
$(call chart2_HrcPartTarget_get_target,$(1)) : $$(SDF)
$(call chart2_HrcPartTarget_get_target,$(1)) : $(SRCDIR)/$(1)
	$$(call gb_SrsPartMergeTarget__command,$$@,$(1),$$<)
endif
endef

define chart2_HrcPartTarget_create_src_dep
$(call gb_SrsPartTarget_get_target,$(2)) : $(call chart2_HrcPartTarget_get_target,$(1))
endef

define chart2_HrcPartTarget_create_src_deps
$(foreach src,$(2),$(eval $(call chart2_HrcPartTarget_create_src_dep,$(1),$(src))))
endef

chart2_HrcTarget__TARGET_DIR = $(WORKDIR)/chart2

chart2_HrcTarget__get_merged_target = $(chart2_HrcTarget__TARGET_DIR)/merged
chart2_HrcTarget__get_unmerged_target = $(chart2_HrcTarget__TARGET_DIR)/unmerged

ifeq ($(strip $(WITH_LANG)),)
chart2_HrcTarget_get_target = $(call chart2_HrcTarget__get_unmerged_target,$(1))
chart2_HrcTarget__get_update_target = $(call chart2_HrcTarget__get_merged_target,$(1))
else
chart2_HrcTarget_get_target = $(call chart2_HrcTarget__get_merged_target,$(1))
chart2_HrcTarget__get_update_target = $(call chart2_HrcTarget__get_unmerged_target,$(1))
endif

chart2_HrcTarget__get_inc_dir = $(chart2_HrcTarget__TARGET_DIR)/inc

chart2_HrcTarget_get_clean_target = $(WORKDIR)/Clean/chart2/$(1)

define chart2_HrcTarget_HrcTarget
$(call chart2_HrcTarget_get_target,$(1)) : PARTS :=
$(call chart2_HrcTarget_get_clean_target,$(1)) : PARTS :=
$(call chart2_HrcTarget_get_target,$(1)) : $(call chart2_HrcTarget__get_update_target,$(1))
	mkdir -p $$(dir $$@) && touch $$@
$(call chart2_HrcTarget__get_update_target,$(1)) : $(gb_Helper_DUMMY)
	mkdir -p $$(dir $$@) && touch $$@
endef

$(call chart2_HrcTarget_get_clean_target,%) : $(gb_Helper_DUMMY)
	$(call gb_Output_announce,$*,$(false),HRC,4)
	rm -f $(foreach part,$(PARTS),$(call chart2_HrcPartTarget_get_target,$(part))) \
		$(call chart2_HrcTarget__get_merged_target,$@) \
		$(call chart2_HrcTarget__get_unmerged_target,$@)

define chart2_HrcTarget_add_file
$(call chart2_HrcPartTarget_HrcPartTarget,$(2))
$(call chart2_HrcTarget_get_target,$(1)) : PARTS += $(2)
$(call chart2_HrcTarget_get_clean_target,$(1)) : PARTS += $(2)
$(call chart2_HrcTarget_get_target,$(1)) : $(call chart2_HrcPartTarget_get_target,$(2))
$(call chart2_HrcPartTarget_get_target,$(2)) : $(call chart2_HrcTarget__get_update_target,$(1))
ifneq ($(strip $(WITH_LANG)),)
$(call chart2_HrcPartTarget_get_target,$(2)) : SDF := $(gb_SrsPartMergeTarget_SDFLOCATION)$(dir $(2))localize.sdf
endif
endef

define chart2_HrcTarget_add_files
$(foreach file,$(2),$(eval $(call chart2_HrcTarget_add_file,$(1),$(file))))
endef

define chart2_HrcTarget_register
$(call gb_SrsTarget_get_target,$(2)) :| $(call chart2_HrcTarget_get_target,$(1))
$(call gb_SrsTarget_get_clean_target,$(2)) : $(call chart2_HrcTarget_get_clean_target,$(1))
endef

$(eval $(call chart2_HrcTarget_HrcTarget,chart2_hrc))

$(eval $(call chart2_HrcTarget_register,chart2_hrc,chart2/res))

$(eval $(call chart2_HrcTarget_add_files,chart2_hrc,\
    chart2/source/controller/dialogs/res_DataLabel_tmpl.hrc \
    chart2/source/controller/dialogs/res_ErrorBar_tmpl.hrc \
    chart2/source/controller/dialogs/res_LegendPosition_tmpl.hrc \
    chart2/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc \
    chart2/source/controller/dialogs/res_Titlesx_tmpl.hrc \
    chart2/source/controller/dialogs/res_Trendline_tmpl.hrc \
))

$(eval $(call chart2_HrcPartTarget_create_src_deps,\
    chart2/source/controller/dialogs/res_DataLabel_tmpl.hrc,\
    chart2/source/controller/dialogs/dlg_InsertDataLabel.src \
    chart2/source/controller/dialogs/tp_DataLabel.src \
))

$(eval $(call chart2_HrcPartTarget_create_src_deps,\
    chart2/source/controller/dialogs/res_ErrorBar_tmpl.hrc,\
    chart2/source/controller/dialogs/dlg_InsertErrorBars.src \
    chart2/source/controller/dialogs/tp_ErrorBars.src \
))

$(eval $(call chart2_HrcPartTarget_create_src_deps,\
    chart2/source/controller/dialogs/res_LegendPosition_tmpl.hrc,\
    chart2/source/controller/dialogs/dlg_InsertLegend.src \
    chart2/source/controller/dialogs/tp_LegendPosition.src \
    chart2/source/controller/dialogs/tp_Wizard_TitlesAndObjects.src \
))

$(eval $(call chart2_HrcPartTarget_create_src_deps,\
    chart2/source/controller/dialogs/res_SecondaryAxisCheckBoxes_tmpl.hrc,\
    chart2/source/controller/dialogs/dlg_InsertAxis_Grid.src \
    chart2/source/controller/dialogs/tp_Wizard_TitlesAndObjects.src \
))

$(eval $(call chart2_HrcPartTarget_create_src_deps,\
    chart2/source/controller/dialogs/res_Titlesx_tmpl.hrc,\
    chart2/source/controller/dialogs/dlg_InsertTitle.src \
    chart2/source/controller/dialogs/tp_Wizard_TitlesAndObjects.src \
))

$(eval $(call chart2_HrcPartTarget_create_src_deps,\
    chart2/source/controller/dialogs/res_Trendline_tmpl.hrc,\
    chart2/source/controller/dialogs/dlg_InsertTrendline.src \
    chart2/source/controller/dialogs/tp_Trendline.src \
))

# vim: set noet ts=4 sw=4:
