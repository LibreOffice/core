# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,openldap))

$(eval $(call gb_ExternalProject_use_externals,openldap,openssl))

$(eval $(call gb_ExternalProject_register_targets,openldap,\
	build \
))

openldap_CFLAGS =
ifeq ($(OS),LINUX) # i.e., assuming glibc
# glibc needs at least _XOPEN_SOURCE=500 to declare pthread_getconcurrency and
# pthread_setconcurrency in <pthread.h> (and once that is defined, it also needs either
# _DEFUALT_SOURCE (glibc >= 2.19) or the deprecated _BSD_SOURCE (glibc <= 2.18) to be defined
# explicitly, so that e.g. u_char is declared in <sys/types.h>):
openldap_CFLAGS = -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -D_BSD_SOURCE
endif

openldap_LDFLAGS = $(call gb_ExternalProject_get_link_flags,openldap)
ifeq ($(SYSTEM_OPENSSL),)
openldap_LDFLAGS += -L$(gb_UnpackedTarball_workdir)/openssl
endif
ifeq ($(OS),LINUX)
openldap_LDFLAGS += -pthread
endif

$(call gb_ExternalProject_get_state_target,openldap,build) :
	$(call gb_Trace_StartRange,openldap,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure \
			--disable-slapd \
			--with-pic \
			--with-tls=openssl \
			--without-cyrus-sasl \
			--disable-shared \
			--enable-static \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(CROSS_COMPILING), \
				--with-yielding_select=yes \
				ac_cv_func_memcmp_working=yes \
			) \
			$(if $(SYSTEM_OPENSSL), \
				CPPFLAGS="$(CPPFLAGS) $(OPENSSL_CFLAGS)" CFLAGS="$(CFLAGS) $(openldap_CFLAGS) $(OPENSSL_CFLAGS) $(call gb_ExternalProject_get_build_flags,openldap)" LDFLAGS="$(LDFLAGS) $(openldap_LDFLAGS) $(OPENSSL_LIBS)" \
				, \
				CPPFLAGS="$(CPPFLAGS) -I$(gb_UnpackedTarball_workdir)/openssl/include" \
				CFLAGS="$(CFLAGS) $(openldap_CFLAGS) $(call gb_ExternalProject_get_build_flags,openldap) -I$(gb_UnpackedTarball_workdir)/openssl/include" \
			) \
			$(if $(openldap_LDFLAGS),LDFLAGS="$(LDFLAGS) $(openldap_LDFLAGS)") \
		&& MAKEFLAGS= && $(MAKE) \
	)
	$(call gb_Trace_EndRange,openldap,EXTERNAL)


# vim: set noet sw=4 ts=4:
