# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,postgresql))

$(eval $(call gb_ExternalProject_use_unpacked,postgresql,postgresql))

$(eval $(call gb_ExternalProject_use_package,postgresql,openldap))

$(eval $(call gb_ExternalProject_register_targets,postgresql,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

$(call gb_ExternalProject_get_state_target,postgresql,build) :
	cd $(EXTERNAL_WORKDIR)/src \
	&& unset MAKEFLAGS \
	&& nmake -f win32.mak USE_SSL=1 USE_LDAP=1 \
	&& touch $@
else

$(call gb_ExternalProject_get_state_target,postgresql,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		--without-readline --disable-shared --with-openssl --with-ldap \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		$(if $(filter YES,$(WITH_KRB5)),--with-krb5) \
		$(if $(filter YES,$(WITH_GSSAPI)),--with-gssapi) \
		$(if $(filter NO,$(SYSTEM_OPENLDAP)),CPPFLAGS="-I$(OUTDIR)/inc/openldap" LDFLAGS="-L$(OUTDIR)/lib" EXTRA_LDAP_LIBS="-llber -lssl3 -lsmime3 -lnss3 -lnssutil3 -lplds4 -lplc4 -lnspr4") \
	&& cd src/interfaces/libpq \
	&& unset MAKEFLAGS \
	&& $(GNUMAKE) -j$(EXTMAXPROCESS) all-static-lib libpq-flags.mk \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
