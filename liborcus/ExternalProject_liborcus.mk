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

# Must be built with debug GNU C++ library if --enable-dbgutil has
# caused the LO code to be built thusly.

# The LIBS setting for Android is needed to get the orcus-xml-dump
# executable to build successfully. We obviously don't actually need
# that executable on Android, but we don't want to bother with
# patching out building it for Android.

#$(if $(filter MSC,$(COM)),CPPFLAGS+="-DBOOST_ALL_NO_LIB") CXXFLAGS+="$(BOOST_CXXFLAGS))

liborcus_LIBS=
ifeq ($(SYSTEM_ZLIB),YES)
liborcus_LIBS+=-lz
endif
ifeq ($(SYSTEM_BOOST),YES)
liborcus_LIBS+=$(BOOST_SYSTEM_LIB)
endif
ifeq ($(OS),ANDROID)
liborcus_LIBS+=-lgnustl_shared -lm
endif

liborcus_CPPCLAGS=$(CPPFLAGS)
ifeq ($(COM),MSC)
liborcus_CPPFLAGS+=-DBOOST_ALL_NO_LIB
endif
ifeq ($(SYSTEM_ZLIB),NO)
liborcus_CPPFLAGS+=-I$(OUTDIR)/inc/external/zlib
endif
ifneq (,$(filter LINUX FREEBSD OPENBSD NETBSD DRAGONFLY ANDROID,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
liborcus_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

liborcus_CXXFLAGS=$(CXXFLAGS)
ifeq ($(COM),MSC)
liborcus_CXXFLAGS+=$(BOOST_CXXFLAGS)
endif
ifeq ($(SYSTEM_BOOST),NO)
liborcus_CXXFLAGS+=-I$(WORKDIR)/UnpackedTarball/boost
endif

$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(liborcus_LIBS),LIBS='$(liborcus_LIBS)') \
		$(if $(liborcus_CXXFLAGS),CXXFLAGS='$(liborcus_CXXFLAGS)') \
		$(if $(liborcus_CPPFLAGS),CPPFLAGS='$(liborcus_CPPFLAGS)') \
		$(if $(filter YES,$(SYSTEM_BOOST)),LDFLAGS='$(BOOST_LDFLAGS)') \
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

# vim: set noet sw=4 ts=4:
