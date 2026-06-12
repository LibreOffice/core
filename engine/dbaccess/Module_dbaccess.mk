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
	UIConfig_dbbrowser \
	UIConfig_dbtdata \
))

ifneq ($(OS),iOS)
# screenshots
$(eval $(call gb_Module_add_screenshot_targets,dbaccess,\
    CppunitTest_dbaccess_dialogs_test \
))

endif


endif

# vim: set noet sw=4 ts=4:
