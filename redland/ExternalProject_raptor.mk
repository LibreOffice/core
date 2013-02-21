# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,raptor))

$(eval $(call gb_ExternalProject_use_unpacked,raptor,raptor))

$(eval $(call gb_ExternalProject_use_external,raptor,libxml2))

$(eval $(call gb_ExternalProject_register_targets,raptor,\
	build \
))

ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,raptor,build):
	cd $(EXTERNAL_WORKDIR) \
	&& CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
	LDFLAGS="-Wl,--no-undefined -Wl,--enable-runtime-pseudo-reloc-v2 -Wl,--export-all-symbols $(subst ;, -L,$(ILIB))" \
	LIBXML2LIB="$(if $(filter YES,$(SYSTEM_LIBXML)),$(LIBXML_LIBS),-lxml2)" \
	XSLTLIB="$(if $(filter YES,$(SYSTEM_LIBXSLT)),$(LIBXSLT_LIBS),-lxslt)" \
	OBJDUMP="$(HOST_PLATFORM)-objdump" \
	./configure --disable-static --enable-shared --disable-gtk-doc --with-openssl-digests \
	--with-xml-parser=libxml --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" \
	--without-bdb --without-sqlite --without-mysql --without-postgresql --without-threestore \
	--with-regex-library=posix --with-decimal=none --with-www=xml \
	--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --target=$(HOST_PLATFORM) \
	lt_cv_cc_dll_switch="-shared" \
	&& $(MAKE) \
	&& touch $@
else
$(call gb_ExternalProject_get_state_target,raptor,build):
	cd $(EXTERNAL_WORKDIR) \
	&& $(if $(filter IOS,$(OS)),LIBS="-liconv") \
	CFLAGS="$(if $(debug),-g,-O) $(if $(filter TRUE,$(DISABLE_DYNLOADING)),-fvisibility=hidden) \
	$(if $(filter GCCLINUXPOWERPC64,$(COM)$(OS)$(CPUNAME)),-mminimal-toc)" \
	LDFLAGS="-L$(OUTDIR)/lib \
		$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib" -Wl$(COMMA)-noinhibit-exec) \
		$(if $(SYSBASE),$(if $(filter LINUX SOLARIS,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
	CPPFLAGS="-I$(OUTDIR)/inc/external $(if $(SYSBASE),-I$(SYSBASE)/usr/include)" \
	./configure --disable-gtk-doc --with-threads --with-openssl-digests \
	--with-xml-parser=libxml --enable-parsers="rdfxml ntriples turtle trig guess rss-tag-soup" \
	--without-bdb --without-sqlite --without-mysql --without-postgresql \
	--without-threestone --with-regex-library=posix --with-decimal=none \
	--with-www=xml \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
        $(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
	$(if $(filter IOS ANDROID,$(OS)),--disable-shared,--disable-static) \
	$(if $(filter NO,$(SYSTEM_LIBXSLT)),--with-xslt-config=$(OUTDIR)/bin/xslt-config) \
	$(if $(filter NO,$(SYSTEM_LIBXML)), \
		LIBXML2LIB=-lxml2 \
		--with-xml2-config=$(OUTDIR)/bin/xml2-config) \
	&& $(MAKE) \
	&& touch $@
endif
# vim: set noet sw=4 ts=4:
