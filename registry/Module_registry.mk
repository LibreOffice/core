# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,registry))

gb_registry_can_build_exe = $(if $(or $(DISABLE_DYNLOADING),$(ENABLE_MACOSX_SANDBOX)),,$(true))

$(eval $(call gb_Module_add_targets,registry,\
	Library_reg \
    $(if $(call gb_registry_can_build_exe), \
			Executable_regmerge \
			Executable_regview \
		StaticLibrary_registry_helper \
    ) \
))

# vim:set noet sw=4 ts=4:
