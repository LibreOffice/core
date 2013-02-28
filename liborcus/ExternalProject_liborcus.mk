# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,liborcus))

$(eval $(call gb_ExternalProject_use_unpacked,liborcus,orcus))

$(eval $(call gb_ExternalProject_use_external,liborcus,boost_headers))

$(eval $(call gb_ExternalProject_register_targets,liborcus,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& export BOOST_LIB_DIR=$(OUTDIR)/lib \
		&& $(COMPATH)/vcpackages/vcbuild.exe liborcus-static-nozip.vcproj "Release|Win32" \
	,vsprojects/liborcus-static-nozip)
else ifeq ($(VCVER),100)
$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& export BOOST_LIB_DIR=$(OUTDIR)/lib \
		&& MSBuild.exe liborcus-static-nozip.vcxproj /p:Configuration=Release /p:OutDir=Release/ /p:TargetName=orcus /p:WholeProgramOptimization=no \
	,vsprojects/liborcus-static-nozip)
else
$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		export BOOST_INCLUDE_DIR=$(call gb_UnpackedTarball_get_dir,boost) \
		&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
		&& export BOOST_LIB_DIR=$(OUTDIR)/lib \
		&& MSBuild.exe liborcus-static-nozip.vcxproj /p:PlatformToolset=v110 /p:VisualStudioVersion=11.0 /p:Configuration=Release /p:OutDir=Release/ /p:TargetName=orcus /p:WholeProgramOptimization=no \
	,vsprojects/liborcus-static-nozip)
endif

else

# must be built with debug STL if --enable-dbgutil
$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter ANDROID,$(OS)),LIBS='-lgnustl_shared -lm' CPPFLAGS="-D_GLIBCXX_HAS_GTHREADS") \
		./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--without-libzip \
			--disable-debug \
			--disable-spreadsheet-model \
			--disable-werror \
			$(if $(filter LINUX FREEBSD OPENBSD NETBSD DRAGONFLY ANDROID,$(OS)),$(if $(gb_ENABLE_DBGUTIL),CPPFLAGS=-D_GLIBCXX_DEBUG)) \
			$(if $(filter NO,$(SYSTEM_BOOST)),CXXFLAGS=-I$(call gb_UnpackedTarball_get_dir,boost),CXXFLAGS=$(BOOST_CPPFLAGS) LDFLAGS=$(BOOST_LDFLAGS)) \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
