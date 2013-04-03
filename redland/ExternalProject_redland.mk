# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,redland))

$(eval $(call gb_ExternalProject_use_unpacked,redland,redland))

$(eval $(call gb_ExternalProject_use_packages,redland, \
    raptor \
    rasqal \
))

$(eval $(call gb_ExternalProject_register_targets,redland,\
	build \
))

# note: this can intentionally only build against internal raptor/rasqal

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,redland,build):
	$(call gb_ExternalProject_run,build,\
		CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
		CPPFLAGS="-I$(OUTDIR)/inc/external" \
		LDFLAGS="-Wl$(COMMA)--no-undefined -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 -Wl$(COMMA)--export-all-symbols -L$(OUTDIR)/lib" \
		OBJDUMP="$(HOST_PLATFORM)-objdump" \
		PKG_CONFIG="" \
		RAPTOR2_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,raptor)/src" \
		RAPTOR2_LIBS="-L$(OUTDIR)/lib -lraptor2  $(if $(filter YES,$(SYSTEM_LIBXML)),$(LIBXML_LIBS),-lxml2)" \
		RASQAL_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,rasqal)/src" \
		RASQAL_LIBS="-L$(OUTDIR)/lib -lrasqal" \
		./configure --disable-static --disable-gtk-doc \
			--disable-modular \
			--without-threads \
			--without-bdb --without-sqlite --without-mysql \
			--without-postgresql --without-threestore --without-virtuoso \
			--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			lt_cv_cc_dll_switch="-shared" \
		&& $(MAKE) \
	)
else
$(call gb_ExternalProject_get_state_target,redland,build):
	$(call gb_ExternalProject_run,build,\
		CFLAGS="$(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden)" \
		LDFLAGS="-L$(OUTDIR)/lib \
		$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib") \
		$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
		CPPFLAGS="-I$(OUTDIR)/inc/external $(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
		PKG_CONFIG="" \
		RAPTOR2_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,raptor)/src" \
		RAPTOR2_LIBS="-L$(OUTDIR)/lib -lraptor2  $(if $(filter YES,$(SYSTEM_LIBXML)),$(LIBXML_LIBS),-lxml2)" \
		RASQAL_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,rasqal)/src" \
		RASQAL_LIBS="-L$(OUTDIR)/lib -lrasqal" \
		./configure --disable-gtk-doc \
			--disable-modular \
			--without-threads \
			--without-bdb --without-sqlite --without-mysql \
			--without-postgresql --without-threestone --without-virtuoso \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SOLARENV)/bin/macosx-change-install-names.pl shl OOO \
			$(gb_Package_SOURCEDIR_redland)/librdf/.libs/librdf-lo.$(RASQAL_MAJOR).dylib) \
	)
endif

# vim: set noet sw=4 ts=4:
