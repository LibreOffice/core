# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,openldap))

$(eval $(call gb_ExternalProject_use_externals,openldap,nss3))

$(eval $(call gb_ExternalProject_register_targets,openldap,\
	build \
))

openldap_LDFLAGS =
ifeq ($(SYSTEM_NSS),)
openldap_LDFLAGS += -L$(call gb_UnpackedTarball_get_dir,nss)/dist/out/lib \
    $(if $(filter AIX,$(OS)),-Wl$(COMMA)-brtl)
endif
# Help openldap's configure determine that it needs -lpthread even if libasan.so
# contains a pthread_create override:
ifneq ($(filter -fsanitize=address,$(CC)),)
openldap_LDFLAGS += -pthread
endif

$(call gb_ExternalProject_get_state_target,openldap,build) :
	$(call gb_Trace_StartRange,openldap,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure \
			--disable-slapd \
			--with-pic \
			--with-tls=moznss \
			--without-cyrus-sasl \
			--disable-shared \
			--enable-static \
			$(if $(CROSS_COMPILING), \
				--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
				--with-yielding_select=yes \
				ac_cv_func_memcmp_working=yes \
			) \
			$(if $(SYSTEM_NSS), \
				CPPFLAGS="$(CPPFLAGS) $(NSS_CFLAGS)" CFLAGS="$(CFLAGS) $(NSS_CFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))" LDFLAGS="$(LDFLAGS) $(NSS_LIBS)" \
				, \
				CPPFLAGS="$(CPPFLAGS) -I$(call gb_UnpackedTarball_get_dir,nss)/dist/public/nss -I$(call gb_UnpackedTarball_get_dir,nss)/dist/out/include" \
				CFLAGS="$(CFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) -I$(call gb_UnpackedTarball_get_dir,nss)/dist/public/nss -I$(call gb_UnpackedTarball_get_dir,nss)/dist/out/include" \
			) \
			$(if $(openldap_LDFLAGS),LDFLAGS="$(LDFLAGS) $(openldap_LDFLAGS)") \
		&& MAKEFLAGS= && $(MAKE) \
	)
	$(call gb_Trace_EndRange,openldap,EXTERNAL)


# vim: set noet sw=4 ts=4:
