# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,basic))

ifneq ($(filter SCRIPTING,$(BUILD_TYPE)),)

$(eval $(call gb_Module_add_l10n_targets,basic,\
       AllLangMoTarget_sb \
))

$(eval $(call gb_Module_add_check_targets,basic,\
	CppunitTest_basic_scanner \
	CppunitTest_basic_macros \
))

endif

$(eval $(call gb_Module_add_targets,basic,\
	Library_sb \
))

# vim: set noet sw=4 ts=4:
