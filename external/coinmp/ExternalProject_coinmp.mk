# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,coinmp))

$(eval $(call gb_ExternalProject_register_targets,coinmp,\
	build \
))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	$(call gb_ExternalProject_run,build,\
		MSBuild.exe CoinMP.sln /t:Build \
			/p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
			/p:Platform=Win32 \
			$(if $(filter 100,$(VCVER)),/p:PlatformToolset=v100 /p:VisualStudioVersion=10.0) \
			$(if $(filter 110,$(VCVER)),/p:PlatformToolset=v110 /p:VisualStudioVersion=11.0) \
	,CoinMP/MSVisualStudio/v9)

else
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	+$(call gb_ExternalProject_run,build,\
		./configure COIN_SKIP_PROJECTS="Data/Sample" \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
