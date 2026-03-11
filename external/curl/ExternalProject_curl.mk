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

ifeq ($(OS),EMSCRIPTEN)
curl_CPPFLAGS := $(gb_EMSCRIPTEN_CPPFLAGS)
else
curl_CPPFLAGS :=
endif
curl_LDFLAGS := $(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN)

ifneq ($(OS),ANDROID)
ifneq ($(SYSBASE),)
curl_CPPFLAGS += -I$(SYSBASE)/usr/include
curl_LDFLAGS += -L$(SYSBASE)/usr/lib
endif
endif

# use --with-openssl on all platforms
$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_Trace_StartRange,curl,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure \
			--disable-ftp --enable-http --enable-ipv6 \
			--without-libidn2 --without-libpsl --without-librtmp \
			--without-libssh2 --without-nghttp2 \
			--without-libssh --without-brotli \
			--without-ngtcp2 --without-quiche \
			--without-zstd --without-libgsasl \
			$(if $(WITH_GSSAPI),--with-gssapi,--without-gssapi) \
			--disable-ipfs --disable-mqtt --disable-ares \
			--disable-dict --disable-file --disable-gopher --disable-imap \
			--disable-ldap --disable-ldaps --disable-manual --disable-pop3 \
			--disable-rtsp --disable-smb --disable-smtp --disable-telnet  \
			--disable-tftp  \
			$(if $(ENABLE_OPENSSL),--with-openssl$(if $(SYSTEM_OPENSSL),,="$(gb_UnpackedTarball_workdir)/openssl")) \
			$(if $(filter iOS MACOSX,$(OS)),--with-apple-sectrust) \
			$(if $(filter LINUX,$(OS)),--without-ca-bundle --without-ca-path) \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			$(if $(ENABLE_DEBUG),--enable-debug) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter MACOSX,$(OS)),CFLAGS='$(CFLAGS) \
				-mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)') \
			$(if $(ENABLE_EMSCRIPTEN_PROXY_POSIX_SOCKETS),--disable-socketpair) \
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

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_Trace_StartRange,curl,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(CMAKE) . \
			$(if $(filter 17.%,$(VCVER)),-G "Visual Studio 17 2022") \
			$(if $(filter 18.%,$(VCVER)),-G "Visual Studio 18 2026") \
			-A $(gb_MSBUILD_PLATFORM) \
			-DBUILD_SHARED_LIBS=ON \
			-DENABLE_IPV6=ON \
			-DCURL_WINDOWS_SSPI=ON \
			-DCURL_USE_SCHANNEL=ON \
			-DCURL_USE_LIBPSL=OFF \
			-DCURL_ZLIB=ON \
			-DZLIB_INCLUDE_DIR=$(gb_UnpackedTarball_workdir)/zlib/ \
			-DZLIB_LIBRARY=$(gb_StaticLibrary_WORKDIR)/zlib.lib \
			-DCURL_DISABLE_DICT=ON \
			-DCURL_DISABLE_FILE=ON \
			-DCURL_DISABLE_FTP=ON \
			-DCURL_DISABLE_GOPHER=ON \
			-DCURL_DISABLE_IMAP=ON \
			-DCURL_DISABLE_IPFS=ON \
			-DCURL_DISABLE_LDAP=ON \
			-DCURL_DISABLE_LDAPS=ON \
			-DCURL_DISABLE_MQTT=ON \
			-DCURL_DISABLE_POP3=ON \
			-DCURL_DISABLE_RTSP=ON \
			-DCURL_DISABLE_SMB=ON \
			-DCURL_DISABLE_SMTP=ON \
			-DCURL_DISABLE_TELNET=ON \
			-DCURL_DISABLE_TFTP=ON \
		&& $(CMAKE) --build . --config $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) \
	)
	$(call gb_Trace_EndRange,curl,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
