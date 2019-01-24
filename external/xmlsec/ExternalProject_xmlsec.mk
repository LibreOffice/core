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

$(eval $(call gb_ExternalProject_register_targets,xmlsec,\
	build \
))

ifeq ($(OS),WNT)

$(eval $(call gb_ExternalProject_use_nmake,xmlsec,build))

$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		cscript /e:javascript configure.js crypto=mscng xslt=no iconv=no static=no \
			lib=$(call gb_UnpackedTarball_get_dir,libxml2)/win32/bin.msvc \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes cruntime=/MDd) \
			cflags="-arch:SSE $(SOLARINC) -I$(WORKDIR)/UnpackedTarball/libxml2/include -I$(WORKDIR)/UnpackedTarball/icu/source/i18n -I$(WORKDIR)/UnpackedTarball/icu/source/common" \
		&& nmake \
	,win32)

else

$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter iOS MACOSX,$(OS)),ACLOCAL="aclocal -I $(SRCDIR)/m4/mac") \
		$(if $(filter AIX,$(OS)),ACLOCAL="aclocal -I /opt/freeware/share/aclocal") \
		autoreconf \
		&& ./configure \
			--with-pic --disable-shared --disable-crypto-dl --without-libxslt --without-gnutls --without-gcrypt --disable-apps --disable-docs \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			CFLAGS="$(CFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS)) $(gb_VISIBILITY_FLAGS)" \
			--without-openssl \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(SYSTEM_NSS),,$(if $(filter MACOSX,$(OS)),--disable-pkgconfig)) \
			$(if $(SYSTEM_NSS),,NSPR_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,nss)/dist/out/include" NSPR_LIBS="-L$(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib -lnspr4") \
			$(if $(SYSTEM_NSS),,NSS_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,nss)/dist/public/nss" NSS_LIBS="-L$(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib -lsmime3 -lnss3 -lnssutil3") \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(SYSBASE),CFLAGS="-I$(SYSBASE)/usr/include" \
			LDFLAGS="-L$(SYSBASE)/usr/lib $(if $(filter-out LINUX FREEBSD,$(OS)),",-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN)) \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
