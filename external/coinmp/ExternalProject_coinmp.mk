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
			$(if $(filter 150,$(VCVER)),/p:PlatformToolset=v141 /p:VisualStudioVersion=15.0 /ToolsVersion:15.0) \
			$(if $(filter 150-10,$(VCVER)-$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION)) \
	,CoinMP/MSVisualStudio/v9)

else
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	+$(call gb_ExternalProject_run,build,\
		./configure COIN_SKIP_PROJECTS="Data/Sample" \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
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
			$(EXTERNAL_WORKDIR)/Cbc/src/.libs/libCbc.3.8.8.dylib \
			$(EXTERNAL_WORKDIR)/Cbc/src/.libs/libCbcSolver.3.8.8.dylib \
			$(EXTERNAL_WORKDIR)/Cgl/src/.libs/libCgl.1.8.5.dylib \
			$(EXTERNAL_WORKDIR)/Clp/src/.libs/libClp.1.12.6.dylib \
			$(EXTERNAL_WORKDIR)/Clp/src/OsiClp/.libs/libOsiClp.1.12.6.dylib \
			$(EXTERNAL_WORKDIR)/CoinMP/src/.libs/libCoinMP.1.7.6.dylib \
			$(EXTERNAL_WORKDIR)/CoinUtils/src/.libs/libCoinUtils.3.9.11.dylib \
			$(EXTERNAL_WORKDIR)/Osi/src/Osi/.libs/libOsi.1.11.5.dylib) \
	)

endif

# vim: set noet sw=4 ts=4:
