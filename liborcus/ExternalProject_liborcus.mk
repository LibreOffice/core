# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,liborcus))

$(eval $(call gb_ExternalProject_use_autoconf,liborcus,build))

$(eval $(call gb_ExternalProject_use_externals,liborcus, \
    boost_headers \
    boostsystem \
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
else
liborcus_LIBS+=-L$(gb_StaticLibrary_WORKDIR) -lboostsystem
endif
ifeq ($(OS),ANDROID)
liborcus_LIBS+=-lgnustl_shared -lm
endif

liborcus_CPPCLAGS=$(CPPFLAGS)
ifeq ($(COM),MSC)
liborcus_CPPFLAGS+=-DBOOST_ALL_NO_LIB
endif
ifeq ($(SYSTEM_ZLIB),NO)
liborcus_CPPFLAGS+=$(ZLIB_CFLAGS)
endif
#
# OSes that use the GNU C++ library need to use -D_GLIBCXX_DEBUG in
# sync with the rest of LibreOffice, i.e. depending on
# --enable-dbgutil. Note that although Android doesn't use the GNU C
# library (glibc), the NDK does offer the GNU C++ library as one of
# the C++ libraries available, and we use it.
#
ifneq (,$(filter ANDROID DRAGONFLY FREEBSD LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
liborcus_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

liborcus_CXXFLAGS=$(CXXFLAGS)
liborcus_LDFLAGS=$(LDFLAGS)
ifeq ($(COM),MSC)
liborcus_CXXFLAGS+=$(BOOST_CXXFLAGS)
endif
ifeq ($(SYSTEM_BOOST),NO)
liborcus_CXXFLAGS+=-I$(WORKDIR)/UnpackedTarball/boost
else
liborcus_LDFLAGS+=$(BOOST_LDFLAGS)
endif
ifneq (,$(PTHREAD_LIBS))
liborcus_LDFLAGS+=$(PTHREAD_LIBS)
endif

$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(liborcus_LIBS),LIBS='$(liborcus_LIBS)') \
		$(if $(liborcus_CXXFLAGS),CXXFLAGS='$(liborcus_CXXFLAGS)') \
		$(if $(liborcus_CPPFLAGS),CPPFLAGS='$(liborcus_CPPFLAGS)') \
		$(if $(liborcus_LDFLAGS),LDFLAGS='$(liborcus_LDFLAGS)') \
		./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			$(if $(filter TRUE,$(ENABLE_DEBUG)),--enable-debug,--disable-debug) \
			--disable-spreadsheet-model \
			--disable-werror \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter NO,$(SYSTEM_BOOST)),--with-boost-system=boostsystem) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
