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
	Zip_smoketestdoc \
))

ifneq ($(ENABLE_JAVA),)
ifneq ($(filter EXTENSIONS,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_targets,smoketest,\
	Extension_TestExtension \
	Jar_TestExtension \
))
endif
endif

ifneq (MACOSX/TRUE,$(OS)/$(ENABLE_MACOSX_SANDBOX))
ifneq ($(filter EXTENSIONS,$(BUILD_TYPE)),)
$(eval $(call gb_Module_add_subsequentcheck_targets,smoketest,\
	CppunitTest_smoketest \
))
endif
endif

endif

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Module_add_targets,smoketest,\
	Jar_smoketest \
))
endif

# vim: set noet sw=4 ts=4:
