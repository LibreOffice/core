# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,cppunit))

$(eval $(call gb_ExternalProject_use_unpacked,cppunit,cppunit))

$(eval $(call gb_ExternalProject_register_targets,cppunit,\
	build \
))


ifeq ($(OS)$(COM),WNTMSC)
ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,cppunit,build) :
	cd $(EXTERNAL_WORKDIR)/src/cppunit \
	&& $(COMPATH)/vcpackages/vcbuild.exe cppunit_dll.vcproj "Release|Win32" \
	&& cd ../DllPlugInTester \
	&& $(COMPATH)/vcpackages/vcbuild.exe DllPlugInTester.vcproj "Release|Win32" \
	&& touch $@
else
$(call gb_ExternalProject_get_state_target,cppunit,build) :
	cd $(EXTERNAL_WORKDIR)/src/cppunit \
	&& msbuild.exe cppunit_dll.vcxproj /p:Configuration=Release \
	$(if $(filter 110,$(VCVER)),/p:PlatformToolset=v110) \
	&& cd ../DllPlugInTester \
	&& msbuild.exe DllPlugInTester.vcxproj /p:Configuration=Release \
	$(if $(filter 110,$(VCVER)),/p:PlatformToolset=v110) \
	&& touch $@
endif
else
$(call gb_ExternalProject_get_state_target,cppunit,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		--disable-dependency-tracking \
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		--disable-doxygen \
		--disable-html-docs \
		--disable-latex-docs \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		$(if $(filter-out WNT,$(OS)),,CXXFLAGS="-mthreads" LDFLAGS="-Wl,--enable-runtime-pseudo-reloc-v2") \
		$(if $(filter SOLARIS,$(OS)),LIBS="-lm") \
		$(if $(filter ANDROID,$(OS)),LIBS="-lgnustl_shared -lm") \
		CXXFLAGS="$(if $(filter GCC,$(COM)),$(if $(filter LINUX FREEBSD OPENBSD NETBSD DRAGONFLY ANDROID,$(OS)),$(if $(filter-out full,$(PRODUCT)),-D_GLIBCXX_DEBUG))) \
		$(if $(debug),-g)" \
	&& cd src \
	&& $(MAKE) \
	&& touch $@
endif

# vim: set noet sw=4 ts=4:
