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
	$(if $(ENABLE_LDAP),openldap) \
	openssl \
))

$(eval $(call gb_ExternalProject_register_targets,postgresql,\
	build \
))

ifeq ($(OS),WNT)

$(eval $(call gb_ExternalProject_use_nmake,postgresql,build))

$(call gb_ExternalProject_get_state_target,postgresql,build) :
	$(call gb_Trace_StartRange,postgresql,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		MSBFLAGS=/p:Platform=$(gb_MSBUILD_PLATFORM) \
		$(PERL) build.pl $(gb_MSBUILD_CONFIG) libpq \
	,src/tools/msvc)
	$(call gb_Trace_EndRange,postgresql,EXTERNAL)

else

postgresql_CPPFLAGS := $(ZLIB_CFLAGS)
postgresql_LDFLAGS  := $(LDFLAGS)

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
	$(call gb_Trace_StartRange,postgresql,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		./configure \
			--without-readline \
			--without-zlib \
			--with-ldap \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(DISABLE_OPENSSL),,--with-openssl \
				$(if $(WITH_GSSAPI),--with-gssapi)) \
				$(if $(ENABLE_LDAP),,--with-ldap=no) \
			CFLAGS="-fPIC" \
			CPPFLAGS="$(postgresql_CPPFLAGS)" \
			LDFLAGS="$(postgresql_LDFLAGS)" \
			$(if $(ENABLE_LDAP),EXTRA_LDAP_LIBS="-llber -lssl3 -lsmime3 -lnss3 -lnssutil3 -lplds4 -lplc4 -lnspr4") \
		&& cd src/interfaces/libpq \
		&& MAKEFLAGS= && $(MAKE) MAKELEVEL=0 all-static-lib)
	$(call gb_Trace_EndRange,postgresql,EXTERNAL)

endif

# vim: set noet sw=4 ts=4:
