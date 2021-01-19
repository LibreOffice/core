# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,postgresql))

$(eval $(call gb_ExternalProject_use_externals,postgresql,\
	openldap \
	openssl \
))

$(eval $(call gb_ExternalProject_register_targets,postgresql,\
	build \
))

ifeq ($(OS),WNT)

$(eval $(call gb_ExternalProject_use_nmake,postgresql,build))

$(call gb_ExternalProject_get_state_target,postgresql,build) :
	$(call gb_ExternalProject_run,build,\
		MSBFLAGS=/p:Platform=$(if $(filter X86_64,$(CPUNAME)),x64,Win32) \
		$(PERL) build.pl $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release) libpq \
	,src/tools/msvc)

else

postgresql_CPPFLAGS := $(ZLIB_CFLAGS)
postgresql_LDFLAGS  :=

ifeq ($(SYSTEM_ZLIB),)
postgresql_LDFLAGS += $(ZLIB_LIBS)
endif

ifeq ($(DISABLE_OPENSSL),)
ifeq ($(SYSTEM_OPENSSL),)
postgresql_CPPFLAGS += -I$(call gb_UnpackedTarball_get_dir,openssl)/include
postgresql_LDFLAGS  += -L$(call gb_UnpackedTarball_get_dir,openssl)/
endif
endif

ifeq ($(SYSTEM_OPENLDAP),)
postgresql_CPPFLAGS += -I$(call gb_UnpackedTarball_get_dir,openldap)/include
postgresql_LDFLAGS  += \
	-L$(call gb_UnpackedTarball_get_dir,openldap)/libraries/libldap_r/.libs \
	-L$(call gb_UnpackedTarball_get_dir,openldap)/libraries/libldap/.libs \
	-L$(call gb_UnpackedTarball_get_dir,openldap)/libraries/liblber/.libs \
	$(if $(SYSTEM_NSS),,\
		-L$(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib) \

endif

# note: as of 13.1, zlib is not needed by libpq
# passing MAKELEVEL=0 is required to find internal headers

$(call gb_ExternalProject_get_state_target,postgresql,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
			--without-readline \
			--without-zlib \
			--with-ldap \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_OPENSSL),,--with-openssl \
				$(if $(WITH_GSSAPI),--with-gssapi)) \
			CFLAGS="-fPIC" \
			CPPFLAGS="$(postgresql_CPPFLAGS)" \
			LDFLAGS="$(postgresql_LDFLAGS)" \
			EXTRA_LDAP_LIBS="-llber -lssl3 -lsmime3 -lnss3 -lnssutil3 -lplds4 -lplc4 -lnspr4" \
		&& cd src/interfaces/libpq \
		&& MAKEFLAGS= && $(MAKE) MAKELEVEL=0 all-static-lib)

endif

# vim: set noet sw=4 ts=4:
