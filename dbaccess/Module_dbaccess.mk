# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,dbaccess))

ifneq (,$(filter DBCONNECTIVITY,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,dbaccess,\
    AllLangResTarget_dba \
    AllLangResTarget_dbmm \
    AllLangResTarget_dbu \
    AllLangResTarget_sdbt \
    $(if $(filter WNT,$(OS)),Executable_odbcconfig) \
    Library_dba \
    Library_dbaxml \
    Library_dbmm \
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

ifeq ($(ENABLE_FIREBIRD_SDBC),TRUE)
$(eval $(call gb_Module_add_check_targets,dbaccess,\
    CppunitTest_dbaccess_firebird_test \
))
endif

# disable test because it still fails in some situations
#    CppunitTest_dbaccess_macros_test \
#


$(eval $(call gb_Module_add_subsequentcheck_targets,dbaccess,\
	JunitTest_dbaccess_complex \
    JunitTest_dbaccess_unoapi \
))

endif

# vim: set noet sw=4 ts=4:
