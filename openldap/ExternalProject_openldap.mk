# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,openldap))

$(eval $(call gb_ExternalProject_use_unpacked,openldap,openldap))

$(eval $(call gb_ExternalProject_register_targets,openldap,\
	build \
))

$(call gb_ExternalProject_get_state_target,openldap,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
			--disable-slapd \
			--with-pic \
			--with-tls=moznss \
			--without-cyrus-sasl \
			--disable-shared \
			--enable-static \
			$(if $(filter YES,$(CROSS_COMPILING)), \
				--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
				--with-yielding_select=yes \
				ac_cv_func_memcmp_working=yes \
			) \
			$(if $(filter YES,$(SYSTEM_NSS)), \
				CPPFLAGS="$(NSS_CFLAGS)" CFLAGS="$(NSS_CFLAGS)" LDFLAGS="$(NSS_LIBS)" \
				, \
				CPPFLAGS="-I$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/public/nss -I$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/out/include" \
				CFLAGS="-I$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/public/nss -I$(call gb_UnpackedTarball_get_dir,nss)/mozilla/dist/out/include" \
				LDFLAGS="-L$(OUTDIR)/lib" \
			) \
		&& MAKEFLAGS= && $(MAKE) \
	)


# vim: set noet sw=4 ts=4:
