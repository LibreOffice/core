# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
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
ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    $(if $(ENABLE_JAVA),CppunitTest_dbaccess_hsqlbinary_import) \
    $(if $(ENABLE_JAVA),CppunitTest_dbaccess_tdf119625) \
    $(if $(ENABLE_JAVA),CppunitTest_dbaccess_tdf126268) \
))

# remove if we have a be file for this
ifeq ($(ENDIANNESS),little)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    CppunitTest_dbaccess_firebird_regression_test \
))
endif
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    CppunitTest_dbaccess_firebird_test \
))
endif

$(eval $(call gb_Module_add_check_targets,dbaccess,\
	CppunitTest_dbaccess_dialog_save \
	CppunitTest_dbaccess_empty_stdlib_save \
	CppunitTest_dbaccess_nolib_save \
	CppunitTest_dbaccess_macros_test \
	CppunitTest_dbaccess_hsqlschema_import \
))

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    CppunitTest_dbaccess_hsqldb_test \
    CppunitTest_dbaccess_RowSetClones \
))
endif

# This runs a suite of performance tests on embedded firebird and HSQLDB.
# Instructions on running the test can be found in qa/unit/embeddedb_performancetest
ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)
ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    CppunitTest_dbaccess_embeddeddb_performancetest \
))
endif
endif

$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	JunitTest_dbaccess_complex \
    JunitTest_dbaccess_unoapi \
))

ifneq ($(DISABLE_PYTHON),TRUE)
ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	PythonTest_dbaccess_python \
))
endif
endif

# screenshots
$(eval $(call gb_Module_add_screenshot_targets,dbaccess,\
    CppunitTest_dbaccess_dialogs_test \
))

endif
endif

# vim: set noet sw=4 ts=4:
