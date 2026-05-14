# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,dbaccess))

$(eval $(call gb_Module_add_targets,dbaccess,\
	Library_dba \
	Library_dbahsql \
))

$(eval $(call gb_Module_add_l10n_targets,dbaccess,\
	AllLangMoTarget_dba \
))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,dbaccess,\
	$(if $(filter WNT,$(OS)),Executable_odbcconfig) \
	Library_dbaxml \
	Library_dbu \
	Library_sdbt \
	UIConfig_dbaccess \
	UIConfig_dbapp \
	UIConfig_dbbrowser \
	UIConfig_dbquery \
	UIConfig_dbrelation \
	UIConfig_dbtable \
	UIConfig_dbtdata \
))

ifneq ($(OS),iOS)
ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
	CppunitTest_dbaccess_dialog_save \
	CppunitTest_dbaccess_empty_stdlib_save \
	CppunitTest_dbaccess_nolib_save \
	CppunitTest_dbaccess_hsqlschema_import \
))
endif

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    CppunitTest_dbaccess_hsqldb_test \
    CppunitTest_dbaccess_RowSetClones \
    CppunitTest_dbaccess_CRMDatabase_test \
))
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	JunitTest_dbaccess_complex \
    JunitTest_dbaccess_unoapi \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	PythonTest_dbaccess_python \
))
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,dbaccess,\
    CppunitTest_dbaccess_dialogs_test \
))

endif

$(eval $(call gb_Module_add_uicheck_targets,dbaccess,\
    UITest_edit_field \
    UITest_query \
))

endif

# vim: set noet sw=4 ts=4:
