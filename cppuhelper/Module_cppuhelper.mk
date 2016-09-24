# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cppuhelper))

$(eval $(call gb_Module_add_targets,cppuhelper,\
	InternalUnoApi_cppuhelper \
	Library_cppuhelper \
	StaticLibrary_findsofficepath \
))

$(eval $(call gb_Module_add_check_targets,cppuhelper,\
	CppunitTest_cppuhelper_cppu_ifcontainer \
	CppunitTest_cppuhelper_cppu_unourl \
))

# CppunitTest_cppuhelper_qa_weak depends on module bridges
$(eval $(call gb_Module_add_subsequentcheck_targets,cppuhelper,\
	CppunitTest_cppuhelper_qa_weak \
))

# vim: set noet sw=4 ts=4:
