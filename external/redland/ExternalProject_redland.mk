# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,redland))

$(eval $(call gb_ExternalProject_use_packages,redland, \
    raptor \
    rasqal \
))

$(eval $(call gb_ExternalProject_register_targets,redland,\
	build \
))

# note: this can intentionally only build against internal raptor/rasqal

$(call gb_ExternalProject_get_state_target,redland,build):
	$(call gb_Trace_StartRange,redland,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
		CFLAGS="$(CFLAGS) $(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden) $(call gb_ExternalProject_get_build_flags,redland) $(gb_EMSCRIPTEN_CPPFLAGS)" \
		LDFLAGS=" \
			$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN") \
			$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
		CPPFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
		PKG_CONFIG="" \
		RAPTOR2_CFLAGS="-I$(gb_UnpackedTarball_workdir)/raptor/src" \
		RAPTOR2_LIBS="-L$(gb_UnpackedTarball_workdir)/raptor/src/.libs -lraptor2 $(LIBXML_LIBS)" \
		RASQAL_CFLAGS="-I$(gb_UnpackedTarball_workdir)/rasqal/src" \
		RASQAL_LIBS="-L$(gb_UnpackedTarball_workdir)/rasqal/src/.libs -lrasqal" \
		$(gb_RUN_CONFIGURE) ./configure --disable-gtk-doc \
			--disable-modular \
			--without-threads \
			--without-bdb --without-sqlite --without-mysql \
			--without-postgresql --without-threestone --without-virtuoso \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(CROSS_COMPILING),$(if $(filter INTEL ARM,$(CPUNAME)),ac_cv_c_bigendian=no)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(ENABLE_DEBUG),--enable-debug) \
			$(if $(DISABLE_DYNLOADING), \
				--enable-static --disable-shared \
			, \
				--enable-shared --disable-static \
			) \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),&& $(PERL) \
			$(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
			$(EXTERNAL_WORKDIR)/src/.libs/librdf-lo.$(REDLAND_MAJOR).dylib) \
	)
	$(call gb_Trace_EndRange,redland,EXTERNAL)

# vim: set noet sw=4 ts=4:
