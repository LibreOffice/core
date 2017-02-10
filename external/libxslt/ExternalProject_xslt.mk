# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,xslt))

$(eval $(call gb_ExternalProject_use_external,xslt,libxml2))

$(eval $(call gb_ExternalProject_register_targets,xslt,\
	build \
))
ifeq ($(OS),WNT)
$(call gb_ExternalProject_get_state_target,xslt,build):
	$(call gb_ExternalProject_run,build,\
		cscript /e:javascript configure.js \
			$(if $(MSVC_USE_DEBUG_RUNTIME),cruntime=/MDd) \
			vcmanifest=yes \
			lib=$(call gb_UnpackedTarball_get_dir,xml2)/win32/bin.msvc \
		&& unset MAKEFLAGS \
		&& LIB="$(ILIB)" nmake \
	,win32)
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,xslt,build):
	$(call gb_ExternalProject_run,build,\
		./configure --without-crypto --without-python \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________OOO) \
			LDFLAGS="$(if $(filter LINUX FREEBSD,$(OS)),-Wl$(COMMA)-z$(COMMA)origin -Wl$(COMMA)-rpath$(COMMA)\\"\$$\$$ORIGIN" -Wl$(COMMA)-noinhibit-exec) \
			$(if $(SYSBASE),$(if $(filter SOLARIS LINUX,$(OS)),-L$(SYSBASE)/lib -L$(SYSBASE)/usr/lib -lpthread -ldl))" \
			$(if $(SYSBASE),CPPFLAGS="-I$(SYSBASE)/usr/include") \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)), \
			$(if $(filter IOS,$(OS)),LIBS="-liconv") \
			--disable-shared,--disable-static) \
			$(if $(SYSTEM_LIBXML),,--with-libxml-src=$(call gb_UnpackedTarball_get_dir,xml2)) \
		&& chmod 777 xslt-config \
		&& $(MAKE) \
		$(if $(filter MACOSX,$(OS)),\
			&& $(PERL) $(SRCDIR)/solenv/bin/macosx-change-install-names.pl shl OOO \
				$(gb_Package_SOURCEDIR_xslt)/libxslt/.libs/libxslt.1.dylib \
		) \
	)
endif

# vim: set noet sw=4 ts=4:
