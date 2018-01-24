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
curl_LDFLAGS := $(if $(filter LINUX FREEBSD,$(OS)),"-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib)

ifneq ($(SYSBASE),)
curl_CPPFLAGS += -I$(SYSBASE)/usr/include
curl_LDFLAGS += -L$(SYSBASE)/usr/lib
endif

# there are 2 include paths, the other one is passed to --with-nss below
ifeq ($(SYSTEM_NSS),NO)
curl_CPPFLAGS += -I$(call gb_UnpackedTarball_get_dir,nss)/dist/public/nss
endif

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_ExternalProject_run,build,\
		CPPFLAGS="$(curl_CPPFLAGS)" \
		LDFLAGS=$(curl_LDFLAGS) \
		./configure \
			$(if $(filter IOS MACOSX,$(OS)),\
				--with-darwinssl,\
				$(if $(ENABLE_NSS),--with-nss$(if $(SYSTEM_NSS),,="$(call gb_UnpackedTarball_get_dir,nss)/dist/out"),--without-nss)) \
			--without-ssl --without-gnutls --without-polarssl --without-cyassl --without-axtls --without-mbedtls \
			--enable-ftp --enable-http --enable-ipv6 \
			--without-libidn2 --without-libpsl --without-librtmp \
			--without-libssh2 --without-metalink --without-nghttp2 \
			--without-libssh --without-brotli \
			--disable-ares \
			--disable-dict --disable-file --disable-gopher --disable-imap \
			--disable-ldap --disable-ldaps --disable-manual --disable-pop3 \
			--disable-rtsp --disable-smb --disable-smtp --disable-telnet  \
			--disable-tftp  \
			$(if $(filter LINUX,$(OS)),--without-ca-bundle --without-ca-path) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
			$(if $(filter TRUE,$(ENABLE_DEBUG)),--enable-debug) \
		&& cd lib \
		&& $(MAKE) \
	)

else ifeq ($(OS)$(COM),WNTGCC)

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_ExternalProject_run,build,\
		PATH=$(OUTDIR)/bin:$$PATH ./configure --with-nss --without-ssl --enable-ftp --enable-ipv6 --disable-http --disable-gopher \
			--disable-file --disable-ldap --disable-telnet --disable-dict --build=i586-pc-mingw32 --host=i586-pc-mingw32 \
			$(if $(filter TRUE,$(ENABLE_DEBUG)),--enable-debug) \
			CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
			LIBS="-lws2_32 -lwinmm $(if $(filter YES,$(MINGW_SHARED_GXXLIB)),$(MINGW_SHARED_LIBSTDCPP))" \
			LDFLAGS="$(patsubst ;, -L,$(ILIB))" \
			CPPFLAGS="$(INCLUDE)" OBJDUMP="objdump" \
		&& cd lib \
		&& $(MAKE) \
	)

else ifeq ($(COM),MSC)

$(call gb_ExternalProject_get_state_target,curl,build):
	$(call gb_ExternalProject_run,build,\
		CC="$(shell cygpath -w $(filter-out -%,$(CC))) $(filter -%,$(CC))" \
		MAKEFLAGS= LIB="$(ILIB)" nmake -f Makefile.vc \
			mode=dll \
			VC=11 \
			$(if $(filter X86_64,$(CPUNAME)),MACHINE=x64,MACHINE=x86) \
			GEN_PDB=$(if $(gb_SYMBOL),yes,no) \
			DEBUG=$(if $(MSVC_USE_DEBUG_RUNTIME),yes,no) \
			ENABLE_IPV6=yes \
			ENABLE_SSPI=yes \
			ENABLE_WINSSL=yes \
	,winbuild)

endif

# vim: set noet sw=4 ts=4:
