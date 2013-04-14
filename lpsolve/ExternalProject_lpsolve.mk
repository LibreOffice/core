# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,lpsolve))

$(eval $(call gb_ExternalProject_use_unpacked,lpsolve,lpsolve))

$(eval $(call gb_ExternalProject_register_targets,lpsolve,\
	build \
))

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(call gb_ExternalProject_get_state_target,lpsolve,build):
	$(call gb_ExternalProject_run,build,\
		$(if $(filter WNT,$(OS_FOR_BUILD)), \
		$(if $(filter YES,$(MINGW_SHARED_GCCLIB)) lpsolve_LDFLAGS="-shared-libgcc") \
		$(if $(filter YES,$(MINGW_SHARED_GXXLIB)) lpsolve_LIBS="$(MINGW_SHARED_LIBSTDCPP)") \
		cmd /c cgcc.bat, sh ccc) \
	,lpsolve55)
else # $(COM)!=GCC
$(call gb_ExternalProject_get_state_target,lpsolve,build):
	$(call gb_ExternalProject_run,build,\
		LIB="$(ILIB)" RUNTIME_FLAG="$(if $(MSVC_USE_DEBUG_RUNTIME),/MDd,/MD)" \
		cmd /c cvc6.bat \
	,lpsolve55)
endif # $(COM)
else # $(OS)!=WNT
$(call gb_ExternalProject_get_state_target,lpsolve,build):
	$(call gb_ExternalProject_run,build,\
		sh $(if $(filter MACOSX,$(OS)),ccc.osx, \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),ccc.static, \
		$(if $(filter AIXGCC,$(OS)$(COM)),ccc.aix.gcc, \
		ccc))) \
	,lpsolve55)
endif # $(OS)
# vim: set noet sw=4 ts=4:
