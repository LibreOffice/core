# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xmlsec))

$(eval $(call gb_ExternalProject_use_externals,xmlsec,\
    libxml2 \
    $(if $(ENABLE_NSS),nss3,$(if $(ENABLE_OPENSSL),openssl)) \
))

$(eval $(call gb_ExternalProject_register_targets,xmlsec,\
	build \
))

# note: it's possible to use XSLT in XML signatures - that appears to be a
# really bad idea from a security point of view though, because it will run
# an XSLT script supplied as untrusted input, and XSLT implementations
# tend to have extension functions, and some of these trivially allow
# running arbitrary code... so investigate the situation with libxslt
# before enabling it here; hopefully nobody uses XSLT in practice anyway.

ifeq ($(OS),WNT)

$(eval $(call gb_ExternalProject_use_nmake,xmlsec,build))

$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_Trace_StartRange,xmlsec,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		cscript /e:javascript configure.js crypto=mscng xslt=no iconv=no static=no \
			lib=$(gb_UnpackedTarball_workdir)/libxml2/win32/bin.msvc \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes cruntime=/MDd) \
			cflags="$(SOLARINC) -I$(WORKDIR)/UnpackedTarball/libxml2/include -I$(WORKDIR)/UnpackedTarball/icu/source/i18n -I$(WORKDIR)/UnpackedTarball/icu/source/common" \
		&& nmake \
	,win32)
	$(call gb_Trace_EndRange,xmlsec,EXTERNAL)

else

$(call gb_ExternalProject_get_state_target,xmlsec,build) :
	$(call gb_Trace_StartRange,xmlsec,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter iOS MACOSX,$(OS_FOR_BUILD)),ACLOCAL="aclocal -I $(SRCDIR)/m4/mac") \
		autoreconf \
		&& $(gb_RUN_CONFIGURE) ./configure \
			--with-pic --disable-shared --disable-crypto-dl --without-libxslt --without-gnutls --without-gcrypt --disable-apps --disable-docs --disable-pedantic \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
			CFLAGS="$(CFLAGS) $(call gb_ExternalProject_get_build_flags,xmlsec) $(gb_VISIBILITY_FLAGS)" \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(ENABLE_NSS), \
				--without-openssl \
				$(if $(SYSTEM_NSS),, \
					$(if $(filter MACOSX,$(OS_FOR_BUILD)),--disable-pkgconfig) \
					NSPR_CFLAGS="-I$(gb_UnpackedTarball_workdir)/nss/dist/out/include" NSPR_LIBS="-L$(gb_UnpackedTarball_workdir)/nss/dist/out/lib -lnspr4" \
					NSS_CFLAGS="-I$(gb_UnpackedTarball_workdir)/nss/dist/public/nss" NSS_LIBS="-L$(gb_UnpackedTarball_workdir)/nss/dist/out/lib -lsmime3 -lnss3 -lnssutil3" \
				), \
				$(if $(ENABLE_OPENSSL), \
					$(if $(SYSTEM_OPENSSL),, \
						OPENSSL_CFLAGS="-I$(gb_UnpackedTarball_workdir)/openssl/include" \
						OPENSSL_LIBS="-L$(gb_UnpackedTarball_workdir)/openssl -lcrypto -lssl" \
					), \
					--without-openssl) \
			) \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(SYSBASE),CFLAGS="-I$(SYSBASE)/usr/include" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,xmlsec) -L$(SYSBASE)/usr/lib $(if $(filter-out LINUX FREEBSD,$(OS)),",-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN)) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,xmlsec,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
