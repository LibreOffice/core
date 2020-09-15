# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,cppunit))

$(eval $(call gb_ExternalProject_register_targets,cppunit,\
	build \
))

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,cppunit,build) :
	$(call gb_Trace_StartRange,cppunit,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
	    PROFILEFLAGS="$(gb_MSBUILD_CONFIG) \
		/p:Platform=$(gb_MSBUILD_PLATFORM) \
			$(if $(filter 160,$(VCVER)),/p:PlatformToolset=v142 /p:VisualStudioVersion=16.0 /ToolsVersion:Current) \
			$(if $(filter 10,$(WINDOWS_SDK_VERSION)),/p:WindowsTargetPlatformVersion=$(UCRTVERSION))" \
		&& msbuild.exe cppunit_dll.vcxproj /p:Configuration=$${PROFILEFLAGS}  \
		&& cd ../DllPlugInTester \
		&& msbuild.exe DllPlugInTester.vcxproj /p:Configuration=$${PROFILEFLAGS} \
	,src/cppunit)
	$(call gb_Trace_EndRange,cppunit,EXTERNAL)
else

cppunit_CXXFLAGS=$(CXXFLAGS)

ifneq (,$(filter ANDROID DRAGONFLY FREEBSD iOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
cppunit_CXXFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

ifneq (,$(call gb_LinkTarget__symbols_enabled,cppunit))
cppunit_CXXFLAGS+=-g
endif

$(call gb_ExternalProject_get_state_target,cppunit,build) :
	$(call gb_Trace_StartRange,cppunit,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		./configure \
			--disable-dependency-tracking \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			--disable-doxygen \
			--disable-html-docs \
			--disable-latex-docs \
			--disable-werror \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________NONE) \
			$(if $(filter WNT,$(OS)),LDFLAGS="-Wl$(COMMA)--enable-runtime-pseudo-reloc-v2") \
			$(if $(filter SOLARIS,$(OS)),LIBS="-lm") \
			$(if $(filter ANDROID,$(OS)),LIBS="$(gb_STDLIBS)") \
			CXXFLAGS="$(cppunit_CXXFLAGS)" \
		&& cd src \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,cppunit,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
