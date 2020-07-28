# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwps))

$(eval $(call gb_ExternalProject_use_autoconf,libwps,build))

$(eval $(call gb_ExternalProject_register_targets,libwps,\
	build \
))

$(eval $(call gb_ExternalProject_use_externals,libwps,\
	revenge \
))

ifneq (,$(filter ANDROID DRAGONFLY FREEBSD iOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
libwps_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

libwps_CXXFLAGS=$(gb_CXXFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))

libwps_LDFLAGS=
ifeq ($(OS),LINUX)
ifeq ($(SYSTEM_REVENGE),)
libwps_LDFLAGS+=-Wl,-z,origin -Wl,-rpath,\$$$$ORIGIN
endif
endif

ifeq ($(ENABLE_GDB_INDEX),TRUE)
libwps_LDFLAGS+=-Wl,--gdb-index
libwps_CXXFLAGS+=-ggnu-pubnames
ifneq ($(USE_LD),)
libwps_LDFLAGS += $(USE_LD)
endif
endif

$(call gb_ExternalProject_get_state_target,libwps,build) :
	$(call gb_Trace_StartRange,libwps,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		export PKG_CONFIG="" \
		&& ./configure \
			--with-pic \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
			--with-sharedptr=c++11 \
			--without-docs \
			--disable-tools \
			$(if $(ENABLE_DEBUG),--enable-debug,--disable-debug) \
			--disable-werror \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(libwps_CXXFLAGS),CXXFLAGS='$(libwps_CXXFLAGS)') \
			$(if $(libwps_CPPFLAGS),CPPFLAGS='$(libwps_CPPFLAGS)') \
			$(if $(libwps_LDFLAGS),LDFLAGS='$(libwps_LDFLAGS)') \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(EXTERNAL_WORKDIR)/src/lib/.libs/libwps-0.4.4.dylib \
		) \
	)
	$(call gb_Trace_EndRange,libwps,EXTERNAL)

# vim: set noet sw=4 ts=4:
