# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xml2))

$(eval $(call gb_ExternalProject_register_targets,xml2,\
	build \
))

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(call gb_ExternalProject_get_state_target,xml2,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-ipv6 --without-python --without-zlib \
			--without-lzma \
			--disable-static --without-debug lt_cv_cc_dll_switch="-shared" \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			CC="$(CC) -mthreads $(if $(MINGW_SHARED_GCCLIB),-shared-libgcc)" \
			LIBS="-lws2_32 $(if $(MINGW_SHARED_GXXLIB),$(MINGW_SHARED_LIBSTDCPP))" \
			LDFLAGS="-Wl$(COMMA)--no-undefined -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2" \
			OBJDUMP=objdump \
		&& $(MAKE) \
	)
else # COM=MSC
$(call gb_ExternalProject_get_state_target,xml2,build):
	$(call gb_ExternalProject_run,build,\
		cscript configure.js \
			iconv=no sax1=yes $(if $(MSVC_USE_DEBUG_RUNTIME),cruntime=/MDd) \
		&& unset MAKEFLAGS \
		&& LIB="$(ILIB)" nmake \
	,win32)
endif
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,xml2,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-ipv6 --without-python --without-zlib --with-sax1 \
			--without-lzma \
			$(if $(debug),--with-run-debug) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________URELIB) \
			LDFLAGS="$(if $(SYSBASE),-L$(SYSBASE)/usr/lib)" \
			CFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include) $(if $(debug),-g)" \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		&& $(MAKE) \
	)
endif

# vim: set noet sw=4 ts=4:
