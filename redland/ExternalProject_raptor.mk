# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,raptor))

$(eval $(call gb_ExternalProject_use_external,raptor,libxml2))

$(eval $(call gb_ExternalProject_register_targets,raptor,\
	build \
))

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,raptor,build):
	$(call gb_ExternalProject_run,build,\
		CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
		LDFLAGS="-Wl$(COMMA)--no-undefined -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 -Wl$(COMMA)--export-all-symbols $(subst ;, -L,$(ILIB))" \
		OBJDUMP="$(HOST_PLATFORM)-objdump" \
		$(if $(and $(filter YES,$(SYSTEM_LIBXML)),$(filter GCC,$(COM))),PATH="$(MINGW_SYSROOT)/bin:$$PATH") \
		./configure --disable-static --enable-shared --disable-gtk-doc \
			--enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" \
			--with-www=xml \
			--without-xslt-config \
			--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --target=$(HOST_PLATFORM) \
			lt_cv_cc_dll_switch="-shared" \
			$(if $(filter NO,$(SYSTEM_LIBXML)),--with-xml2-config=$(call gb_UnpackedTarball_get_dir,xml2)/xml2-config) \
		&& $(MAKE) \
	)
else
$(call gb_ExternalProject_get_state_target,raptor,build):
	$(call gb_ExternalProject_run,build,\
		$(if $(filter IOS,$(OS)),LIBS="-liconv") \
		CFLAGS="$(if $(debug),-g,-O) $(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden) \
			$(if $(filter GCCLINUXPOWERPC64,$(COM)$(OS)$(CPUNAME)),-mminimal-toc)" \
		LDFLAGS=" \
			$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib") \
			$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
		CPPFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
		./configure --disable-gtk-doc \
			 --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" \
			--with-www=xml \
			--without-xslt-config \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
			$(if $(filter NO,$(SYSTEM_LIBXML)),--with-xml2-config=$(call gb_UnpackedTarball_get_dir,xml2)/xml2-config) \
		&& $(MAKE) \
	)
endif

# vim: set noet sw=4 ts=4:
