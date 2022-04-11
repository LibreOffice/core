# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,lcms2))

$(eval $(call gb_ExternalProject_register_targets,lcms2,\
	build \
))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,lcms2,build):
	$(call gb_Trace_StartRange,lcms2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		MSBuild.exe lcms2_DLL.vcxproj \
			$(gb_MSBUILD_CONFIG_AND_PLATFORM) /p:TargetName=lcms2 \
			/p:PlatformToolset=$(VCTOOLSET) /p:VisualStudioVersion=$(VCVER) /ToolsVersion:Current \
			$(if $(filter 10,$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION)) \
	,Projects/VC2019/lcms2_DLL)
	$(call gb_Trace_EndRange,lcms2,EXTERNAL)
else
$(call gb_ExternalProject_get_state_target,lcms2,build):
	$(call gb_Trace_StartRange,lcms2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --without-jpeg --without-tiff --with-pic \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(CROSS_COMPILING),$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no)) \
			CPPFLAGS=" $(SOLARINC)" \
			CFLAGS='$(CFLAGS) $(call gb_ExternalProject_get_build_flags,lcms2)' \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& cd src \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,lcms2,EXTERNAL)
endif
# vim: set noet sw=4 ts=4:
