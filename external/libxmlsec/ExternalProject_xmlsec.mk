# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xmlsec))

$(eval $(call gb_ExternalProject_use_external,xmlsec,libxml2))

$(eval $(call gb_ExternalProject_use_external,xmlsec,nss3))

$(eval $(call gb_ExternalProject_use_external,xmlsec,openssl))

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
			$(if $(SYSTEM_NSS),,--disable-pkgconfig) \
			CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
			LDFLAGS="-Wl$(COMMA)--no-undefined $(ILIB:;= -L)" \
			LIBS="$(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_SHARED__LIBSTDCPP))" \
			lt_cv_deplibs_check_method=pass_all \
		&& $(MAKE) \
	)

else
$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		cscript configure.js crypto=mscrypto xslt=no iconv=no static=no \
			lib=$(call gb_UnpackedTarball_get_dir,xml2)/win32/bin.msvc \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes) \
		&& unset MAKEFLAGS \
		&& LIB="$(ILIB)" nmake \
	,win32)
endif

else

$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter MACOSX,$(OS)),ACLOCAL="aclocal -I $(SRCDIR)/m4/mac") \
		$(if $(filter AIX,$(OS)),ACLOCAL="aclocal -I /opt/freeware/share/aclocal") \
		autoreconf \
		&& ./configure \
			--with-pic --disable-shared --disable-crypto-dl --without-libxslt --without-gnutls \
			$(if $(or $(filter-out ANDROID,$(OS)),$(DISABLE_OPENSSL)),--without-openssl,--with-openssl=$(call gb_UnpackedTarball_get_dir,openssl)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(SYSTEM_NSS),,$(if $(filter MACOSX,$(OS)),--disable-pkgconfig)) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(SYSTEM_LIBXML),,LIBXML_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,xml2)/include" LIBXML_LIBS="-L$(call gb_UnpackedTarball_get_dir,xml2)/.libs -lxml2")\
			$(if $(SYSBASE),CFLAGS="-I$(SYSBASE)/usr/include" \
			LDFLAGS="-L$(SYSBASE)/usr/lib $(if $(filter-out LINUX FREEBSD,$(OS)),,-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib)) \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
