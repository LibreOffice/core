# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# for VERSION
include $(SRCDIR)/external/coinmp/version.mk

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
			$(if $(filter 110,$(VCVER)),/p:PlatformToolset=$(if $(filter 80,$(WINDOWS_SDK_VERSION)),v110,v110_xp) /p:VisualStudioVersion=11.0) \
			$(if $(filter 120,$(VCVER)),/p:PlatformToolset=v120 /p:VisualStudioVersion=12.0 /ToolsVersion:12.0) \
	,CoinMP/MSVisualStudio/v9)

else
$(call gb_ExternalProject_get_state_target,coinmp,build) :
	+$(call gb_ExternalProject_run,build,\
		./configure COIN_SKIP_PROJECTS="Data/Sample" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_DYNLOADING),--disable-shared,$(if $(filter MACOSX,$(OS)),--libdir=/@__________________________________________________OOO)) \
			--enable-dependency-linking F77=unavailable \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl app OOO \
				$(gb_Package_SOURCEDIR_coinmp)/Cbc/src/.libs/libCbc.$(CBC_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/Cbc/src/.libs/libCbcSolver.$(CBCSOLVER_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/Cgl/src/.libs/libCgl.$(CGL_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/Clp/src/.libs/libClp.$(CLP_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/Clp/src/OsiClp/.libs/libOsiClp.$(OSICLP_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/CoinMP/src/.libs/libCoinMP.$(COINMP_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/CoinUtils/src/.libs/libCoinUtils.$(COINUTILS_VERSION).dylib \
				$(gb_Package_SOURCEDIR_coinmp)/Osi/src/Osi/.libs/libOsi.$(OSI_VERSION).dylib \
		) \
	)

endif

# vim: set noet sw=4 ts=4:
