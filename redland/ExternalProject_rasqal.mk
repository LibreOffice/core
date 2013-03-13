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

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,rasqal,build):
	$(call gb_ExternalProject_run,build,\
		CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
		LDFLAGS="-Wl$(COMMA)--no-undefined -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2 -Wl$(COMMA)--export-all-symbols $(subst ;, -L$,$(ILIB))" \
		LIBXML2LIB="$(if $(filter YES,$(SYSTEM_LIBXML)),$(LIBXML_LIBS),-lxml2)" \
		XSLTLIB="$(if $(filter YES,$(SYSTEM_LIBXSLT)),$(LIBXSLT_LIBS),-lxslt)" \
		OBJDUMP="$(HOST_PLATFORM)-objdump" \
		./configure --disable-static --enable-shared --disable-gtk-doc --with-openssl-digests \
			--with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql \
			--without-postgresql --without-threestore \
			--disable-pcre --with-decimal=none --with-www=xml \
			--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) \
			lt_cv_cc_dll_switch="-shared" \
		&& $(MAKE) \
	)
else
$(call gb_ExternalProject_get_state_target,rasqal,build):
	$(call gb_ExternalProject_run,build,\
		CFLAGS="$(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden)" \
		PATH="$(OUTDIR)/bin:$$PATH" \
		LDFLAGS="-L$(OUTDIR)/lib \
		$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib" -Wl$(COMMA)-noinhibit-exec) \
		$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl)) \
		$(if $(filter MACOSXNO,$(OS)$(SYSTEM_LIBXML)),-Wl$(COMMA)-dylib_file$(COMMA)@loader_path/../ure-link/lib/libxml2.2.dylib:$(OUTDIR)/lib/libxml2.2.dylib)" \
		CPPFLAGS="-I$(OUTDIR)/inc/external $(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
		./configure --disable-gtk-doc --with-threads --with-openssl-digests \
			--with-xml-parser=libxml --without-bdb --without-sqlite --without-mysql \
			--without-postgresql --without-threestone --with-regex-library=posix \
			--with-decimal=none --with-www=xml \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
		&& $(MAKE) \
			$(if $(filter MACOSX,$(OS)),&& $(PERL) \
            $(SOLARENV)/bin/macosx-change-install-names.pl shl OOO \
            $(gb_Package_SOURCEDIR_rasqal)/src/.libs/librasqal-lo.1.dylib) \
	)
endif
# vim: set noet sw=4 ts=4:
