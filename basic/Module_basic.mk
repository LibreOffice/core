# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,basic))

ifneq ($(DISABLE_SCRIPTING),TRUE)

$(eval $(call gb_Module_add_targets,basic,\
	AllLangResTarget_sb \
))

$(eval $(call gb_Module_add_check_targets,basic,\
     CppunitTest_basic_scanner \
     CppunitTest_basic_enable \
     CppunitTest_basic_nested_struct \
     CppunitTest_basic_coverage \
     CppunitTest_basic_vba \
))

endif

$(eval $(call gb_Module_add_targets,basic,\
	Library_sb \
	Package_inc \
))

# vim: set noet sw=4 ts=4:
