# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,rasqal))

$(eval $(call gb_ExternalProject_use_unpacked,rasqal,rasqal))

$(eval $(call gb_ExternalProject_use_external,rasqal,libxml2))

$(eval $(call gb_ExternalProject_use_package,rasqal,raptor))

$(eval $(call gb_ExternalProject_register_targets,rasqal,\
	build \
))

# note: this can intentionally only build against internal raptor (not system)

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,rasqal,build):
	$(call gb_ExternalProject_run,build,\
		CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
		LDFLAGS="-Wl$(COMMA)--no-undefined -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 -Wl$(COMMA)--export-all-symbols $(subst ;, -L$,$(ILIB))" \
		OBJDUMP="$(HOST_PLATFORM)-objdump" \
		PKG_CONFIG="" \
		RAPTOR2_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,raptor)/src" \
		RAPTOR2_LIBS="-L$(OUTDIR)/lib -lraptor2" \
		./configure --disable-static --enable-shared --disable-gtk-doc \
			--disable-pcre \
			--with-decimal=none \
			--with-uuid-library=internal \
			--with-digest-library=internal \
			--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			lt_cv_cc_dll_switch="-shared" \
			$(if $(filter NO,$(SYSTEM_LIBXML)),--with-xml2-config=$(call gb_UnpackedTarball_get_dir,xml2)/xml2-config) \
		&& $(MAKE) \
	)
else
$(call gb_ExternalProject_get_state_target,rasqal,build):
	$(call gb_ExternalProject_run,build,\
		CFLAGS="$(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden)" \
		LDFLAGS=" \
			$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-rpath-link$(COMMA)$(OUTDIR)/lib -Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib") \
			$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
		$(if $(SYSBASE),CPPFLAGS="-I$(SYSBASE)/usr/include") \
		PKG_CONFIG="" \
		RAPTOR2_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,raptor)/src" \
		RAPTOR2_LIBS="-L$(OUTDIR)/lib -lraptor2" \
		./configure --disable-gtk-doc \
			--with-regex-library=posix \
			--with-decimal=none \
			--with-uuid-library=internal \
			--with-digest-library=internal \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
			$(if $(filter NO,$(SYSTEM_LIBXML)),--with-xml2-config=$(call gb_UnpackedTarball_get_dir,xml2)/xml2-config) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SOLARENV)/bin/macosx-change-install-names.pl shl OOO \
			$(gb_Package_SOURCEDIR_rasqal)/src/.libs/librasqal-lo.$(RASQAL_MAJOR).dylib) \
	)
endif

# vim: set noet sw=4 ts=4:
