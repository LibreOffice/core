# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xml2))

$(eval $(call gb_ExternalProject_use_unpacked,xml2,xml2))

$(eval $(call gb_ExternalProject_register_targets,xml2,\
	build \
))
ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(call gb_ExternalProject_get_state_target,xml2,build):
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure --disable-ipv6 --without-python --without-zlib \
	--disable-static --without-debug lt_cv_cc_dll_switch="-shared" \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
	LIBS="-lws2_32 $(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_SHARED_LIBSTDCPP))" \
	LDFLAGS="-Wl,--no-undefined -Wl,--enable-runtime-pseudo-reloc-v2" \
	OBJDUMP=objdump \
	&& $(MAKE) \
	&& touch $@
else # COM=MSC
$(call gb_ExternalProject_get_state_target,xml2,build):
	cd $(EXTERNAL_WORKDIR)/win32 \
	&& cscript configure.js iconv=no sax1=yes \
	&& unset MAKEFLAGS \
	&& LIB="$(ILIB)" nmake \
	&& touch $@
endif
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,xml2,build):
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure --disable-ipv6 --without-python --without-zlib --with-sax1 \
	$(if $(debug),--with-mem-debug --with-run-debug) \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	LDFLAGS="$(if $(SYSBASE),-L$(SYSBASE)/usr/lib)" \
	CFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include) $(if $(debug),-g)" \
	$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
	&& $(MAKE) \
	&& touch $@
endif
# vim: set noet sw=4 ts=4:
