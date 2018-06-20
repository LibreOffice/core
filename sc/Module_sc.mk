# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/sc/common_unoapi_tests.mk

$(eval $(call gb_Module_Module,sc))

$(eval $(call gb_Module_add_targets,sc,\
	Library_sc \
	Library_scd \
	Library_scfilt \
	$(call gb_Helper_optional,DESKTOP,Library_scui) \
	$(call gb_Helper_optional,OPENCL,Package_opencl) \
	Package_res_xml \
	UIConfig_scalc \
))

$(eval $(call gb_Module_add_l10n_targets,sc,\
	AllLangMoTarget_sc \
))

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

$(eval $(call gb_Module_add_targets,sc,\
	Library_vbaobj \
))

endif

ifneq ($(OS),IOS)
$(eval $(call gb_Module_add_check_targets,sc,\
	Library_scqahelper \
	$(if $(and $(filter $(COM),MSC),$(MERGELIBS)),, \
		CppunitTest_sc_ucalc) \
	CppunitTest_sc_bugfix_test \
	CppunitTest_sc_filters_test \
	CppunitTest_sc_rangelst_test \
	CppunitTest_sc_range_test \
	CppunitTest_sc_mark_test \
	CppunitTest_sc_core \
	CppunitTest_sc_dataprovider \
	CppunitTest_sc_datatransformation \
	CppunitTest_sc_cache_test \
))

ifneq ($(ENABLE_HEADLESS),TRUE)
ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_check_targets,sc,\
	CppunitTest_sc_tiledrendering \
))
endif
endif

$(eval $(call gb_Module_add_slowcheck_targets,sc, \
	CppunitTest_sc_cond_format_merge \
	CppunitTest_sc_new_cond_format_api \
	CppunitTest_sc_subsequent_filters_test \
	CppunitTest_sc_subsequent_export_test \
	CppunitTest_sc_html_export_test \
	CppunitTest_sc_copypaste \
	CppunitTest_sc_pivottable_filters_test \
))

# Various function tests fail in 32-bit linux_x86 build due to dreaded floating
# point weirdness (x87, registers, compiler optimization, ... whatever),
# disable them until someone finds a real cure.

ifneq ($(filter-out linux_aarch64 linux_x86,$(PLATFORMID)),)
$(eval $(call gb_Module_add_slowcheck_targets,sc, \
	CppunitTest_sc_functions_test_old \
	CppunitTest_sc_database_functions_test \
	CppunitTest_sc_array_functions_test \
	CppunitTest_sc_addin_functions_test \
	CppunitTest_sc_datetime_functions_test \
	CppunitTest_sc_financial_functions_test \
	CppunitTest_sc_information_functions_test \
	CppunitTest_sc_logical_functions_test \
	CppunitTest_sc_mathematical_functions_test \
	CppunitTest_sc_spreadsheet_functions_test \
	CppunitTest_sc_statistical_functions_test \
	CppunitTest_sc_text_functions_test \
))
endif

# Disabled to allow the check tinderbox execute the sd tests
# CppunitTest_sc_chart_regression_test \

$(eval $(call gb_Module_add_subsequentcheck_targets,sc,\
	JunitTest_sc_complex \
	JunitTest_sc_unoapi_1 \
	JunitTest_sc_unoapi_2 \
	JunitTest_sc_unoapi_3 \
	JunitTest_sc_unoapi_4 \
	JunitTest_sc_unoapi_5 \
	JunitTest_sc_unoapi_6 \
	JunitTest_sc_unoapi_7 \
	CppunitTest_sc_opencl_test \
	CppunitTest_sc_parallelism \
	CppunitTest_sc_anchor_test \
	CppunitTest_sc_annotationshapeobj \
	CppunitTest_sc_outlineobj \
	CppunitTest_sc_styleloaderobj \
	CppunitTest_sc_recordchanges \
	CppunitTest_sc_annotationobj \
	CppunitTest_sc_annotationsobj \
	CppunitTest_sc_cellrangeobj \
	$(if $(filter-out $(OS),IOS), \
		CppunitTest_sc_databaserangeobj) \
	CppunitTest_sc_datapilottableobj \
	CppunitTest_sc_datapilotfieldobj \
	CppunitTest_sc_macros_test \
	CppunitTest_sc_namedrangeobj \
	CppunitTest_sc_namedrangesobj \
	CppunitTest_sc_tablesheetobj \
	CppunitTest_sc_tablesheetsobj \
	CppunitTest_sc_editfieldobj_cell \
	CppunitTest_sc_editfieldobj_header \
	CppunitTest_sc_modelobj \
	CppunitTest_sc_check_xcell_ranges_query \
	CppunitTest_sc_check_data_pilot_field \
	CppunitTest_sc_check_data_pilot_table \
	CppunitTest_sc_viewpaneobj \
	CppunitTest_sc_cellobj \
	CppunitTest_sc_cellcursorobj \
	CppunitTest_sc_tabviewobj \
	CppunitTest_sc_arealinkobj \
	CppunitTest_sc_subtotaldescriptorbaseobj \
	CppunitTest_sc_datapilotitemobj \
	CppunitTest_sc_subtotalfieldobj \
	CppunitTest_sc_arealinksobj \
	CppunitTest_sc_cellrangesobj \
	CppunitTest_sc_filterdescriptorbaseobj \
	CppunitTest_sc_tablevalidationobj \
	CppunitTest_sc_tableconditionalentryobj \
	CppunitTest_sc_tableconditionalformatobj \
	CppunitTest_sc_scenariosobj \
	CppunitTest_sc_labelrangesobj \
	CppunitTest_sc_labelrangeobj \
	CppunitTest_sc_headerfootercontentobj \
	CppunitTest_sc_recentfunctionsobj \
	CppunitTest_sc_functionlistobj \
	CppunitTest_sc_datapilottablesobj \
	CppunitTest_sc_databaserangesobj \
	CppunitTest_sc_consolidationdescriptorobj \
	CppunitTest_sc_chart2dataprovider \
	CppunitTest_sc_autoformatobj \
	CppunitTest_sc_importdescriptorbaseobj \
	CppunitTest_sc_documentconfigurationobj \
	CppunitTest_sc_spreadsheetsettings \
	CppunitTest_sc_shapeobj \
	CppunitTest_sc_sheetlinkobj \
	CppunitTest_sc_spreadsheetsettingsobj \
	CppunitTest_sc_ddelinkobj \
	CppunitTest_sc_functiondescriptionobj \
	CppunitTest_sc_sortdescriptorbaseobj \
))

$(eval $(call gb_Module_add_perfcheck_targets,sc,\
	CppunitTest_sc_perfobj \
	CppunitTest_sc_tablesheetobj \
))

ifneq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_Module_add_subsequentcheck_targets,sc,\
	PythonTest_sc_python \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,sc,\
	CppunitTest_sc_screenshots \
))

$(eval $(call gb_Module_add_uicheck_targets,sc,\
	UITest_conditional_format \
	UITest_range_name \
	UITest_hide_cols \
	UITest_autofilter \
	UITest_search_replace \
	UITest_calc_tests \
	UITest_calc_tests2 \
	UITest_calc_tests3 \
	UITest_calc_tests4 \
	UITest_statistics \
	UITest_solver \
	UITest_goalSeek \
))
endif

# vim: set noet sw=4 ts=4:
