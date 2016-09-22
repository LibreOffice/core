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
			/p:Platform=$(if $(filter INTEL,$(CPUNAME)),Win32,x64) \
			$(if $(filter 120,$(VCVER)),/p:PlatformToolset=v120 /p:VisualStudioVersion=12.0 /ToolsVersion:12.0) \
			$(if $(filter 140,$(VCVER)),/p:PlatformToolset=v140 /p:VisualStudioVersion=14.0 /ToolsVersion:14.0) \
	,CoinMP/MSVisualStudio/v9)

else
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	+$(call gb_ExternalProject_run,build,\
		./configure COIN_SKIP_PROJECTS="Data/Sample" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_DYNLOADING),--disable-shared) \
			--disable-bzlib \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			--enable-dependency-linking F77=unavailable \
			$(if $(filter LINUX,$(OS)), \
				'LDFLAGS=-Wl$(COMMA)-z$(COMMA)origin \
					-Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN') \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			$(gb_Package_SOURCEDIR_coinmp)/Cbc/src/.libs/libCbc.3.8.8.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/Cbc/src/.libs/libCbcSolver.3.8.8.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/Cgl/src/.libs/libCgl.1.8.5.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/Clp/src/.libs/libClp.1.12.6.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/Clp/src/OsiClp/.libs/libOsiClp.1.12.6.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/CoinMP/src/.libs/libCoinMP.1.7.6.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/CoinUtils/src/.libs/libCoinUtils.3.9.11.dylib \
			$(gb_Package_SOURCEDIR_coinmp)/Osi/src/Osi/.libs/libOsi.1.11.5.dylib) \
	)

endif

# vim: set noet sw=4 ts=4:
