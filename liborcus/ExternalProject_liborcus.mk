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

$(eval $(call gb_ExternalProject_use_autoconf,liborcus,build))

$(eval $(call gb_ExternalProject_use_externals,liborcus, \
    boost_headers \
    zlib \
))

$(eval $(call gb_ExternalProject_register_targets,liborcus,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

ifeq ($(VCVER),90)
$(call gb_ExternalProject_get_state_target,liborcus,build) :
	export BOOST_INCLUDE_DIR=$(WORKDIR)/UnpackedTarball/boost \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& export BOOST_LIB_DIR=$(OUTDIR)/lib \
	&& cd $(EXTERNAL_WORKDIR)/vsprojects/liborcus-static \
	&& $(COMPATH)/vcpackages/vcbuild.exe liborcus-static.vcproj "Release|Win32" \
	&& cd $(EXTERNAL_WORKDIR)/vsprojects/liborcus-parser-static \
	&& $(COMPATH)/vcpackages/vcbuild.exe liborcus-parser-static.vcproj "Release|Win32" \
	&& touch $@
else
$(call gb_ExternalProject_get_state_target,liborcus,build) :
	export BOOST_INCLUDE_DIR=$(WORKDIR)/UnpackedTarball/boost \
	&& export ZLIB_INCLUDE_DIR=$(OUTDIR)/inc/external/zlib \
	&& export BOOST_LIB_DIR=$(OUTDIR)/lib \
	&& cd $(EXTERNAL_WORKDIR)/vsprojects/liborcus-static \
	&& $(COMPATH)/../Common7/Tools/vcupgrade.exe liborcus-static.vcproj \
	&& MSBuild.exe liborcus-static.vcxproj /p:Configuration=Release /p:OutDir=Release/ /p:TargetName=liborcus-static /p:WholeProgramOptimization=no \
	&& cd $(EXTERNAL_WORKDIR)/vsprojects/liborcus-parser-static \
	&& $(COMPATH)/../Common7/Tools/vcupgrade.exe liborcus-parser-static.vcproj \
	&& MSBuild.exe liborcus-parser-static.vcxproj /p:Configuration=Release /p:OutDir=Release/ /p:TargetName=liborcus-parser-static /p:WholeProgramOptimization=no \
	&& touch $@
endif

else

# Must be built with debug GNU C++ library if --enable-dbgutil has
# caused the LO code to be built thusly.

# The LIBS setting for Android is needed to get the orcus-xml-dump
# executable to build successfully. We obviously don't actually need
# that executable on Android, but we don't want to bother with
# patching out building it for Android.

#$(if $(filter MSC,$(COM)),CPPFLAGS+="-DBOOST_ALL_NO_LIB") CXXFLAGS+="$(BOOST_CXXFLAGS))

$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter ANDROID,$(OS)),LIBS='-lgnustl_shared -lm') \
		$(if $(filter YES,$(SYSTEM_ZLIB)),LIBS+=-lz) \
		$(if $(filter MSC,$(COM)),CPPFLAGS+="-DBOOST_ALL_NO_LIB") \
		$(if $(filter MSC,$(COM)),CXXFLAGS+=$(BOOST_CXXFLAGS)) \
		$(if $(filter NO,$(SYSTEM_ZLIB)),CPPFLAGS+=-I$(OUTDIR)/inc/external/zlib) \
		$(if $(filter NO,$(SYSTEM_BOOST)),CXXFLAGS+=-I$(WORKDIR)/UnpackedTarball/boost) \
		$(if $(filter YES,$(SYSTEM_BOOST)),LDFLAGS=$(BOOST_LDFLAGS)) \
		$(if $(filter LINUX FREEBSD OPENBSD NETBSD DRAGONFLY ANDROID,$(OS)),$(if $(gb_ENABLE_DBGUTIL),CPPFLAGS+=-D_GLIBCXX_DEBUG)) \
		./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--disable-debug \
			--disable-spreadsheet-model \
			--disable-werror \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
