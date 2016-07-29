# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,sc))

$(eval $(call gb_Module_add_targets,sc,\
	Library_sc \
	Library_scd \
	Library_scfilt \
	$(call gb_Helper_optional,DESKTOP,Library_scui) \
))

$(eval $(call gb_Module_add_l10n_targets,sc,\
	AllLangResTarget_sc \
	UIConfig_scalc \
))

ifeq ($(ENABLE_TELEPATHY),TRUE)

$(eval $(call gb_Module_add_targets,sc,\
	CustomTarget_uiconfig \
))

endif

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

$(eval $(call gb_Module_add_targets,sc,\
	Library_vbaobj \
))

endif

$(eval $(call gb_Module_add_check_targets,sc,\
	Library_scqahelper \
	$(if $(and $(filter $(COM),MSC),$(MERGELIBS)),, \
		CppunitTest_sc_ucalc) \
	CppunitTest_sc_bugfix_test \
	CppunitTest_sc_filters_test \
	CppunitTest_sc_rangelst_test \
	CppunitTest_sc_mark_test \
	CppunitTest_sc_core \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_check_targets,sc,\
    CppunitTest_sc_tiledrendering \
))
endif

$(eval $(call gb_Module_add_slowcheck_targets,sc, \
	CppunitTest_sc_condformats \
	CppunitTest_sc_new_cond_format_api \
	CppunitTest_sc_subsequent_filters_test \
	CppunitTest_sc_subsequent_export_test \
	CppunitTest_sc_html_export_test \
	CppunitTest_sc_opencl_test \
	CppunitTest_sc_copypaste \
	CppunitTest_sc_functions_test \
))

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,sc, \
	CppunitTest_sc_screenshots \
))

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
))

$(eval $(call gb_Module_add_perfcheck_targets,sc,\
	CppunitTest_sc_perfobj \
	CppunitTest_sc_tablesheetobj \
))


# vim: set noet sw=4 ts=4:
