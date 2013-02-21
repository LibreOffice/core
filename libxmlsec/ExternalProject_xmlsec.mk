# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xmlsec))

$(eval $(call gb_ExternalProject_use_unpacked,xmlsec,xmlsec))

$(eval $(call gb_ExternalProject_use_external,xmlsec,libxml2))

$(eval $(call gb_ExternalProject_register_targets,xmlsec,\
	build \
))

ifeq ($(OS),WNT)

ifeq ($(COM),GCC)
$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure --build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			--without-libxslt --without-openssl --without-gnutls --disable-crypto-dl \
			$(if $(filter NO,$(SYSTEM_NSS)),--disable-pkgconfig) \
			CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
			LDFLAGS="-Wl,--no-undefined $(ILIB:;= -L)" \
			LIBS="$(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_SHARED__LIBSTDCPP))" \
		&& $(MAKE) \
	)

else
$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		cscript configure.js crypto=mscrypto xslt=no iconv=no static=no \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes) \
		&& unset MAKEFLAGS \
		&& LIB="$(ILIB)" nmake \
	,win32)
endif

else

$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter MACOSX,$(OS)),ACLOCAL="aclocal -I $(SRCDIR)/m4/mac") autoreconf \
		&& ./configure \
			--with-pic --disable-shared --disable-crypto-dl --without-libxslt --without-gnutls \
			$(if $(filter ANDROID,$(OS)),--with-openssl=$(OUTDIR),--without-openssl) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter NO,$(SYSTEM_NSS))$(filter MACOSX,$(OS)),--disable-pkgconfig) \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(SYSBASE),CFLAGS="-I$(SYSBASE)/usr/include" \
			LDFLAGS="-L$(SYSBASE)/usr/lib $(if $(filter-out LINUX FREEBSD,$(OS)),,-Wl,-z,origin -Wl,-rpath,\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib),\
			$(if $(filter-out MACOSX,$(OS)),,LDFLAGS="-Wl,-dylib_file,@executable_path/libnssutil3.dylib:$(OUTDIR)/lib/libnssutil3.dylib")) \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
