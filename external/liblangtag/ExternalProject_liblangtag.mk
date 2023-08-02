# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,liblangtag))

$(eval $(call gb_ExternalProject_use_external,liblangtag,libxml2))

$(eval $(call gb_ExternalProject_use_autoconf,liblangtag,build))

$(eval $(call gb_ExternalProject_register_targets,liblangtag,\
	build \
))

$(call gb_ExternalProject_get_state_target,liblangtag,build):
	$(call gb_Trace_StartRange,liblangtrag,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		MAKE=$(MAKE) $(gb_RUN_CONFIGURE) ./configure --disable-modules --disable-test --disable-introspection --with-pic \
		$(if $(or $(DISABLE_DYNLOADING),$(filter MSC,$(COM))), \
			--disable-shared --enable-static, \
			--enable-shared --disable-static) \
		$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
		$(if $(filter TRUE,$(HAVE_GCC_BUILTIN_ATOMIC)),"lt_cv_has_atomic=yes","lt_cv_has_atomic=no") \
		$(if $(filter -fsanitize=undefined,$(CC)),CC='$(CC) -fno-sanitize=function') \
		CFLAGS='$(CFLAGS) -pthread \
			$(call gb_ExternalProject_get_build_flags,liblangtag)' \
		$(gb_CONFIGURE_PLATFORMS) \
		$(if $(CROSS_COMPILING),$(if $(filter WNT,$(OS)),"lt_cv_c99_vsnprintf=yes" "ac_cv_va_copy=yes","ac_cv_va_copy=no")) \
		LIBXML2_CFLAGS="$(LIBXML_CFLAGS)" \
		LIBXML2_LIBS="$(if $(filter WNT,$(OS)),-L$(call gb_UnpackedTarball_get_dir,libxml2)/win32/bin.msvc -llibxml2,$(LIBXML_LIBS))" \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________URELIB) \
		$(if $(filter-out LINUX FREEBSD,$(OS)),,LDFLAGS="-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath,\\"\$$\$$ORIGIN) \
		$(if $(filter-out SOLARIS,$(OS)),,LDFLAGS="-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-R$(COMMA)\\"\$$\$$ORIGIN) \
		&& $(if $(verbose),V=1) \
		   $(MAKE) \
                LIBO_TUNNEL_LIBRARY_PATH='$(subst ','\'',$(subst $$,$$$$,$(call gb_Helper_extend_ld_path,$(call gb_UnpackedTarball_get_dir,liblangtag)/liblangtag/.libs)))' \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl URELIB \
				$(EXTERNAL_WORKDIR)/liblangtag/.libs/liblangtag.1.dylib \
		) \
	)
	$(call gb_Trace_EndRange,liblangtrag,EXTERNAL)

# vim: set noet sw=4 ts=4:
