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


ifeq ($(OS)$(COM),WNTMSC)
$(call gb_ExternalProject_get_state_target,cppunit,build) :
	$(call gb_ExternalProject_run,build,\
		msbuild.exe cppunit_dll.vcxproj /p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
		$(if $(filter 110,$(VCVER)),/p:PlatformToolset=v110 /p:VisualStudioVersion=11.0) \
		&& cd ../DllPlugInTester \
		&& msbuild.exe DllPlugInTester.vcxproj /p:Configuration=$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
		$(if $(filter 110,$(VCVER)),/p:PlatformToolset=v110 /p:VisualStudioVersion=11.0) \
	,src/cppunit)
else

cppunit_CXXFLAGS=$(CXXFLAGS)

ifneq (,$(filter ANDROID DRAGONFLY FREEBSD IOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
cppunit_CXXFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

ifeq ($(OS)-$(COM),WNT-GCC)
cppunit_CXXFLAGS+=-mthreads
endif

ifneq (,$(debug))
cppunit_CXXFLAGS+=-g
endif

$(call gb_ExternalProject_get_state_target,cppunit,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
			--disable-dependency-tracking \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			--disable-doxygen \
			--disable-html-docs \
			--disable-latex-docs \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter WNT,$(OS)),LDFLAGS="-Wl$(COMMA)--enable-runtime-pseudo-reloc-v2") \
			$(if $(filter SOLARIS,$(OS)),LIBS="-lm") \
			$(if $(filter ANDROID,$(OS)),LIBS="-lgnustl_shared -lm") \
			CXXFLAGS="$(cppunit_CXXFLAGS)" \
		&& cd src \
		&& $(MAKE) \
	)
endif

# vim: set noet sw=4 ts=4:
