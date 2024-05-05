# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,curl))

$(eval $(call gb_ExternalProject_use_externals,curl,\
	$(if $(ENABLE_OPENSSL),openssl) \
	zlib \
))

$(eval $(call gb_ExternalProject_register_targets,curl,\
	build \
))

ifneq ($(OS),WNT)

curl_CPPFLAGS :=
curl_LDFLAGS := $(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN)

ifneq ($(OS),ANDROID)
ifneq ($(SYSBASE),)
curl_CPPFLAGS += -I$(SYSBASE)/usr/include
curl_LDFLAGS += -L$(SYSBASE)/usr/lib
endif
endif

# use --with-secure-transport on macOS >10.5 and iOS to get a native UI for SSL certs for CMIS usage
# use --with-openssl only on platforms other than macOS and iOS
$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_Trace_StartRange,curl,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure \
			--without-amissl --without-bearssl --without-gnutls \
			--without-mbedtls --without-rustls --without-wolfssl \
			--disable-ftp --enable-http --enable-ipv6 \
			--without-libidn2 --without-libpsl --without-librtmp \
			--without-libssh2 --without-nghttp2 \
			--without-libssh --without-brotli \
			--without-ngtcp2 --without-quiche \
			--without-zstd --without-hyper --without-libgsasl --without-gssapi \
			--disable-mqtt --disable-ares \
			--disable-dict --disable-file --disable-gopher --disable-imap \
			--disable-ldap --disable-ldaps --disable-manual --disable-pop3 \
			--disable-rtsp --disable-smb --disable-smtp --disable-telnet  \
			--disable-tftp  \
			$(if $(filter iOS MACOSX,$(OS)),\
				--with-secure-transport,\
				$(if $(ENABLE_OPENSSL),--with-openssl$(if $(SYSTEM_OPENSSL),,="$(gb_UnpackedTarball_workdir)/openssl"))) \
			$(if $(filter LINUX,$(OS)),--without-ca-bundle --without-ca-path) \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			$(if $(ENABLE_DEBUG),--enable-debug) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter MACOSX,$(OS)),CFLAGS='$(CFLAGS) \
				-mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)') \
			$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
			CPPFLAGS='$(curl_CPPFLAGS)' \
			CFLAGS="$(gb_CFLAGS) $(call gb_ExternalProject_get_build_flags,curl)" \
			LDFLAGS='$(call gb_ExternalProject_get_link_flags,curl) $(curl_LDFLAGS)' \
			ZLIB_CFLAGS='$(ZLIB_CFLAGS)' ZLIB_LIBS='$(ZLIB_LIBS)' \
		&& cd lib \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,curl,EXTERNAL)

else ifeq ($(COM),MSC)

$(eval $(call gb_ExternalProject_use_nmake,curl,build))

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_Trace_StartRange,curl,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		nmake -f Makefile.vc \
			mode=dll \
			VC=12 \
			MACHINE=$(gb_MSBUILD_PLATFORM) \
			GEN_PDB=$(if $(call gb_Module__symbols_enabled,curl),yes,no) \
			$(if $(call gb_Module__symbols_enabled,curl),CFLAGS_PDB_VALUE="$(gb_DEBUGINFO_FLAGS)") \
			DEBUG=$(if $(MSVC_USE_DEBUG_RUNTIME),yes,no) \
			ENABLE_IPV6=yes \
			ENABLE_SSPI=yes \
			ENABLE_WINSSL=yes \
			WITH_ZLIB=static \
	,winbuild)
	$(call gb_Trace_EndRange,curl,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
