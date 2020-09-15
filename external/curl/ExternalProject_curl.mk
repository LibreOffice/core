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
	$(if $(ENABLE_NSS),nss3) \
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

# there are 2 include paths, the other one is passed to --with-nss below
ifeq ($(SYSTEM_NSS),)
curl_CPPFLAGS += -I$(call gb_UnpackedTarball_get_dir,nss)/dist/public/nss
endif

# use --with-darwinssl on macOS >10.5 and iOS to get a native UI for SSL certs for CMIS usage
# use --with-nss only on platforms other than macOS and iOS
$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_Trace_StartRange,curl,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		./configure \
			$(if $(filter iOS MACOSX,$(OS)),\
				--with-darwinssl,\
				$(if $(ENABLE_NSS),--with-nss$(if $(SYSTEM_NSS),,="$(call gb_UnpackedTarball_get_dir,nss)/dist/out"),--without-nss)) \
			--without-ssl --without-gnutls --without-polarssl --without-cyassl --without-axtls --without-mbedtls \
			--enable-ftp --enable-http --enable-ipv6 \
			--without-libidn2 --without-libpsl --without-librtmp \
			--without-libssh2 --without-metalink --without-nghttp2 \
			--without-libssh --without-brotli \
			--without-ngtcp2 --without-quiche \
			--disable-ares \
			--disable-dict --disable-file --disable-gopher --disable-imap \
			--disable-ldap --disable-ldaps --disable-manual --disable-pop3 \
			--disable-rtsp --disable-smb --disable-smtp --disable-telnet  \
			--disable-tftp  \
			$(if $(filter LINUX,$(OS)),--without-ca-bundle --without-ca-path) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			$(if $(ENABLE_DEBUG),--enable-debug) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter MACOSX,$(OS)),CFLAGS='$(CFLAGS) \
				-mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)') \
			CPPFLAGS='$(curl_CPPFLAGS)' \
			LDFLAGS='$(curl_LDFLAGS)' \
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
			DEBUG=$(if $(MSVC_USE_DEBUG_RUNTIME),yes,no) \
			ENABLE_IPV6=yes \
			ENABLE_SSPI=yes \
			ENABLE_WINSSL=yes \
	,winbuild)
	$(call gb_Trace_EndRange,curl,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
