# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,testtools))

ifneq ($(CROSS_COMPILING),YES)

$(eval $(call gb_Module_add_targets,testtools,\
	CustomTarget_bridgetest \
	InternalUnoApi_bridgetest \
	InternalUnoApi_performance \
	StaticLibrary_bridgetest \
	Library_cppobj \
	Library_bridgetest \
	Library_constructors \
	Rdb_uno_services \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,testtools,\
	Jar_testComponent \
	CustomTarget_bridgetest_javamaker \
))
endif

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,testtools,\
	CustomTarget_bridgetest_climaker \
))
endif

$(eval $(call gb_Module_add_check_targets,testtools,\
	CustomTarget_uno_test \
))

endif

# vim:set noet sw=4 ts=4:
