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
	AllLangResTarget_sc \
	Library_sc \
	Library_scd \
	Library_scfilt \
	Library_scui \
	UIConfig_scalc \
))

ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Module_add_targets,sc,\
	Library_scqahelper \
))
endif

ifeq ($(ENABLE_TELEPATHY),TRUE)

$(eval $(call gb_Module_add_targets,sc,\
	CustomTarget_uiconfig \
))

endif

ifneq ($(DISABLE_SCRIPTING),TRUE)

$(eval $(call gb_Module_add_targets,sc,\
	Library_vbaobj \
))

endif

$(eval $(call gb_Module_add_check_targets,sc,\
    CppunitTest_sc_ucalc \
    CppunitTest_sc_filters_test \
    CppunitTest_sc_rangelst_test \
))

$(eval $(call gb_Module_add_slowcheck_targets,sc, \
    CppunitTest_sc_subsequent_filters_test \
    CppunitTest_sc_subsequent_export_test \
))

# Disabled to allow the check tinderbox execute the sd tests
# CppunitTest_sc_chart_regression_test \
# FIXME_REMOVE_WHEN_RE_BASE_COMPLETE
# CppunitTest_sc_annotationshapeobj \

$(eval $(call gb_Module_add_subsequentcheck_targets,sc,\
    JunitTest_sc_complex \
    JunitTest_sc_unoapi \
    CppunitTest_sc_outlineobj \
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
))

# vim: set noet sw=4 ts=4:
