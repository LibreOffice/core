# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,smoketest))

ifeq ($(CROSS_COMPILING),)

$(eval $(call gb_Module_add_targets,smoketest,\
	Library_smoketest \
	Package_losmoketest \
	Package_smoketestdoc \
	Zip_smoketestdoc \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,smoketest,\
	Extension_TestExtension \
	Jar_TestExtension \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Module_add_check_targets,smoketest,\
	Executable_libtest \
))
endif

ifneq (MACOSX/YES,$(OS)/$(ENABLE_MACOSX_SANDBOX))
$(eval $(call gb_Module_add_subsequentcheck_targets,smoketest,\
	CppunitTest_smoketest \
))
endif

endif

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,smoketest,\
	Jar_smoketest \
))
endif

# vim: set noet sw=4 ts=4:
