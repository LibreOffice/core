# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libxml2))

$(eval $(call gb_ExternalProject_register_targets,libxml2,\
	build \
))

ifeq ($(OS),WNT)
$(call gb_ExternalProject_use_external_project,libxml2,icu)

$(eval $(call gb_ExternalProject_use_nmake,libxml2,build))

$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_ExternalProject_run,build,\
		cscript /e:javascript configure.js \
			iconv=no icu=yes sax1=yes $(if $(MSVC_USE_DEBUG_RUNTIME),cruntime=/MDd) \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes) \
		&& nmake \
	,win32)
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-ipv6 --without-python --without-zlib --with-sax1 \
			--without-lzma \
			$(if $(debug),--with-run-debug) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________URELIB) \
			LDFLAGS="$(if $(SYSBASE),-L$(SYSBASE)/usr/lib)" \
			CFLAGS="$(if $(SYSBASE),-I$(SYSBASE)/usr/include) $(if $(debug),-g)" \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		&& $(MAKE) libxml2.la xmllint \
	)
endif

# vim: set noet sw=4 ts=4:
