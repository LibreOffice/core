# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,lpsolve))

$(eval $(call gb_ExternalProject_register_targets,lpsolve,\
	build \
))

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,lpsolve,build):
	$(call gb_ExternalProject_run,build,\
		LIB="$(ILIB)" RUNTIME_FLAG="$(if $(MSVC_USE_DEBUG_RUNTIME),/MDd,/MD)" \
		cmd /c cvc6.bat \
	,lpsolve55)
else # $(OS)!=WNT
$(call gb_ExternalProject_get_state_target,lpsolve,build):
	$(call gb_ExternalProject_run,build,\
		CC="$(CC) $(if $(debug),$(gb_COMPILERNOOPTFLAGS) $(gb_DEBUGINFO_FLAGS) $(gb_DEBUG_CFLAGS),$(gb_COMPILEROPTFLAGS))" \
		$(if $(filter MACOSX,$(OS)),EXTRA_LINKFLAGS='-install_name @__________________________________________________OOO/liblpsolve55.dylib') \
		sh -e $(if $(filter MACOSX,$(OS)),ccc.osx, \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),ccc.static, \
		$(if $(filter AIXGCC,$(OS)$(COM)),ccc.aix.gcc, \
		ccc))) \
	,lpsolve55)
endif # $(OS)
# vim: set noet sw=4 ts=4:
