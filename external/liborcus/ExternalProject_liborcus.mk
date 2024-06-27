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
    boost_filesystem \
    boost_iostreams \
    boost_system \
	mdds_headers \
    zlib \
))

$(eval $(call gb_ExternalProject_register_targets,liborcus,\
	build \
))

# Must be built with debug GNU C++ library if --enable-dbgutil has
# caused the LO code to be built thus.

# The LIBS setting for Android is needed to get the orcus-xml-dump
# executable to build successfully. We obviously don't actually need
# that executable on Android, but we don't want to bother with
# patching out building it for Android.

#$(if $(filter MSC,$(COM)),CPPFLAGS+="-DBOOST_ALL_NO_LIB") CXXFLAGS+="$(BOOST_CXXFLAGS))

liborcus_LIBS=
ifneq ($(SYSTEM_ZLIB),)
liborcus_LIBS+=-lz
endif
ifneq ($(SYSTEM_BOOST),)
liborcus_LIBS+=$(BOOST_SYSTEM_LIB) $(BOOST_IOSTREAMS_LIB) $(BOOST_FILESYSTEM_LIB)
else
liborcus_LIBS+=-L$(gb_StaticLibrary_WORKDIR) -lboost_system -lboost_iostreams -lboost_filesystem
endif
ifeq ($(OS),ANDROID)
liborcus_LIBS+=$(gb_STDLIBS)
endif
ifneq ($(HAVE_LIBSTDCPP),)
# fix std::filesystem linker error:
# https://stackoverflow.com/questions/33149878/experimentalfilesystem-linker-error
liborcus_LIBS+=-lstdc++fs
endif

liborcus_CPPFLAGS=$(CPPFLAGS)
ifeq ($(SYSTEM_ZLIB),)
liborcus_CPPFLAGS+=$(ZLIB_CFLAGS)
endif
#
# OSes that use the GNU C++ library need to use -D_GLIBCXX_DEBUG in
# sync with the rest of LibreOffice, i.e. depending on
# --enable-dbgutil. Note that although Android doesn't use the GNU C
# library (glibc), the NDK does offer the GNU C++ library as one of
# the C++ libraries available, and we use it.
#
liborcus_CPPFLAGS+=$(gb_COMPILERDEFS_STDLIB_DEBUG)

liborcus_CXXFLAGS=$(CXXFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_VISIBILITY_FLAGS_CXX) $(CXXFLAGS_CXX11) -DBOOST_SYSTEM_NO_DEPRECATED
liborcus_LDFLAGS=$(LDFLAGS) $(gb_LTOFLAGS)
liborcus_CXXFLAGS+=$(call gb_ExternalProject_get_build_flags,liborcus)
liborcus_LDFLAGS+=$(call gb_ExternalProject_get_link_flags,liborcus)
ifeq ($(COM),MSC)
liborcus_CXXFLAGS+=$(BOOST_CXXFLAGS)
endif
ifeq ($(SYSTEM_BOOST),)
liborcus_CXXFLAGS+=${BOOST_CPPFLAGS}
else
liborcus_LDFLAGS+=$(BOOST_LDFLAGS)
endif
ifneq (,$(PTHREAD_LIBS))
liborcus_LDFLAGS+=$(PTHREAD_LIBS)
endif

ifeq ($(OS),LINUX)
liborcus_LDFLAGS+=-Wl,-z,origin -Wl,-rpath,\$$$$ORIGIN
endif

$(call gb_ExternalProject_get_state_target,liborcus,build) :
	$(call gb_Trace_StartRange,liborcus,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(liborcus_LIBS),LIBS='$(liborcus_LIBS)') \
		$(if $(liborcus_CXXFLAGS),CXXFLAGS='$(liborcus_CXXFLAGS)') \
		$(if $(liborcus_CPPFLAGS),CPPFLAGS='$(liborcus_CPPFLAGS) $(gb_EMSCRIPTEN_CPPFLAGS)') \
		$(if $(liborcus_LDFLAGS),LDFLAGS='$(liborcus_LDFLAGS)') \
		MDDS_CFLAGS='$(MDDS_CFLAGS)' \
		MDDS_LIBS=' ' \
		MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
                       $(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
                       --disable-spreadsheet-model \
                       --without-tools \
                       --without-benchmark \
                       --without-doc-example \
                       --disable-python \
                       --disable-werror \
                       $(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
                       $(if $(SYSTEM_BOOST),,\
				--with-boost=$(WORKDIR)/UnpackedTarball/boost \
				boost_cv_lib_iostreams=yes \
				boost_cv_lib_system=yes \
				boost_cv_lib_filesystem=yes \
			) \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(if $(verbose),V=1) \
		   $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/liborcus/.libs/liborcus-0.18.0.dylib \
				$(EXTERNAL_WORKDIR)/src/parser/.libs/liborcus-parser-0.18.0.dylib \
		) \
	)
	$(call gb_Trace_EndRange,liborcus,EXTERNAL)

# vim: set noet sw=4 ts=4:
