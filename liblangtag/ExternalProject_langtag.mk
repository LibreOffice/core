# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,langtag))

$(eval $(call gb_ExternalProject_use_unpacked,langtag,langtag))
$(eval $(call gb_ExternalProject_use_external,langtag,libxml2))

$(eval $(call gb_ExternalProject_register_targets,langtag,\
	build \
))

$(call gb_ExternalProject_get_state_target,langtag,build):
	$(call gb_ExternalProject_run,build,\
		$(if $(filter MSC,$(COM)), \
		export LIB="$(ILIB)" \
		CC="$(CC) -MD -nologo \
			$(if $(filter TRUE,$(ENABLE_DEBUG)),-Zi) \
			$(SOLARINC)" && ) \
		./configure --disable-modules --disable-test --disable-introspection --disable-shared --enable-static --with-pic \
		$(if $(filter TRUE,$(HAVE_GCC_BUILTIN_ATOMIC)),"lt_cv_has_atomic=yes","lt_cv_has_atomic=no") \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		$(if $(filter NO,$(SYSTEM_LIBXML)),LIBXML2_CFLAGS="-I$(call gb_UnpackedTarball_get_dir,xml2)/include" \
		$(if $(filter MSC,$(COM)),LIBXML2_LIBS="$(OUTDIR)/lib/libxml2.lib",LIBXML2_LIBS="-L$(OUTDIR)/lib -lxml2"),\
		$(if $(filter MACOSX,$(OS)),LIBXML2_CFLAGS="$(LIBXML_CFLAGS)" LIBXML2_LIBS="$(LIBXML_LIBS)")) \
		$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
		$(if $(filter-out LINUX FREEBSD,$(OS)),,LDFLAGS="-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath,\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib) \
		$(if $(filter-out SOLARIS,$(OS)),,LDFLAGS="-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-R$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib) \
		$(if $(filter-out WNTGCC,$(OS)$(COM)),,LDFLAGS="-Wl$(COMMA)--enable-runtime-pseudo-reloc-v2") \
		&& $(MAKE) \
	)
# vim: set noet sw=4 ts=4:
