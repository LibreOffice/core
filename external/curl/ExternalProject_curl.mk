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
	nss3 \
	zlib \
))

$(eval $(call gb_ExternalProject_register_targets,curl,\
	build \
))

ifneq ($(OS),WNT)

curl_CPPFLAGS :=
curl_LDFLAGS := $(if $(filter LINUX FREEBSD,$(OS)),"-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN)

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

# use --with-darwinssl on Mac OS X >10.5 and iOS to get a native UI for SSL certs for CMIS usage
# use --with-nss only on platforms other than Mac OS X and iOS
$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS="$(curl_CPPFLAGS)" \
		LDFLAGS=$(curl_LDFLAGS) \
		./configure \
			$(if $(filter IOS MACOSX,$(OS)),\
				--with-darwinssl,\
				--with-nss$(if $(SYSTEM_NSS),,="$(call gb_UnpackedTarball_get_dir,nss)/dist/out")) \
			--without-ssl --without-gnutls --without-polarssl --without-cyassl --without-axtls \
			--enable-ftp --enable-http --enable-ipv6 \
			--without-libidn2 --without-libpsl --without-librtmp \
			--without-libssh2 --without-metalink --without-nghttp2 \
			--disable-ares \
			--disable-dict --disable-file --disable-gopher --disable-imap \
			--disable-ldap --disable-ldaps --disable-manual --disable-pop3 \
			--disable-rtsp --disable-smb --disable-smtp --disable-telnet  \
			--disable-tftp  \
			$(if $(filter LINUX,$(OS)),--without-ca-bundle --without-ca-path) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			$(if $(ENABLE_DEBUG),--enable-debug) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		&& cd lib \
		&& $(MAKE) \
	)

else ifeq ($(OS)$(COM),WNTGCC)

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_ExternalProject_run,build,\
		./configure --with-nss --without-ssl --enable-ftp --enable-ipv6 --disable-http --disable-gopher \
			--disable-file --disable-ldap --disable-telnet --disable-dict --build=i586-pc-mingw32 --host=i586-pc-mingw32 \
			$(if $(ENABLE_DEBUG),--enable-debug) \
			CC="$(CC) -mthreads $(if $(MINGW_SHARED_GCCLIB),-shared-libgcc)" \
			LIBS="-lws2_32 -lwinmm $(if $(MINGW_SHARED_GXXLIB),$(MINGW_SHARED_LIBSTDCPP))" \
			LDFLAGS="$(patsubst ;, -L,$(ILIB))" \
			CPPFLAGS="$(INCLUDE)" OBJDUMP="objdump" \
		&& cd lib \
		&& $(MAKE) \
	)

else ifeq ($(COM),MSC)

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_ExternalProject_run,build,\
		MAKEFLAGS= LIB="$(ILIB)" nmake -f Makefile.vc12 \
			cfg=$(if $(MSVC_USE_DEBUG_RUNTIME),debug-dll,release-dll) \
			EXCFLAGS="/EHa /D_CRT_SECURE_NO_DEPRECATE /DUSE_WINDOWS_SSPI /D_USING_V110_SDK71_ $(SOLARINC)" $(if $(filter X86_64,$(CPUNAME)),MACHINE=X64) \
	,lib)

endif

# vim: set noet sw=4 ts=4:
