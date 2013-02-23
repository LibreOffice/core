# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xslt))

$(eval $(call gb_ExternalProject_use_unpacked,xslt,xslt))

$(eval $(call gb_ExternalProject_use_external,xslt,libxml2))

$(eval $(call gb_ExternalProject_register_targets,xslt,\
	build \
))
ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(call gb_ExternalProject_get_state_target,xslt,build):
	$(call gb_ExternalProject_run,build,\
		./configure --without-crypto --without-python --disable-static \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			CC="$(CC) -mthreads $(if $(filter YES,$(MINGW_SHARED_GCCLIB)),-shared-libgcc)" \
			$(if $(filter YES,$(MINGW_SHARED_GXXLIB)),LIBS="$(MINGW_SHARED_LIBSTDCPP)") \
			LDFLAGS="-Wl$(COMMA)--no-undefined -Wl$(COMMA)--enable-runtime-pseudo-reloc-v2" \
			OBJDUMP=objdump \
		&& chmod 777 xslt-config \
		&& $(MAKE) \
	)
else # COM=MSC
$(call gb_ExternalProject_get_state_target,xslt,build):
	$(call gb_ExternalProject_run,build,\
		cscript configure.js \
		&& unset MAKEFLAGS \
		&& LIB="$(ILIB)" nmake \
	,win32)
endif
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,xslt,build):
	$(call gb_ExternalProject_run,build,\
		./configure --without-crypto --without-python \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			LDFLAGS="$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN:'\'\$$\$$ORIGIN/../ure-link/lib" -Wl$(COMMA)-noinhibit-exec) \
			$(if $(SYSBASE),$(if $(filter SOLARIS LINUX,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
			$(if $(SYSBASE),CPPFLAGS="-I$(SYSBASE)/usr/include") \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)), \
			$(if $(filter IOS,$(OS)),LIBS="-liconv") \
			--disable-shared,--disable-static) \
			$(if $(filter NO,$(SYSTEM_LIBXML)),--with-libxml-prefix=$(OUTDIR) LIBXML2LIB=-lxml2) \
		&& chmod 777 xslt-config \
		&& $(MAKE) \
	)
endif

# vim: set noet sw=4 ts=4:
