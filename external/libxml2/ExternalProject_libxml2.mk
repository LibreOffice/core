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

ifeq ($(OS),EMSCRIPTEN)
$(call gb_ExternalProject_use_external_project,libxml2,icu)
endif

ifeq ($(OS),WNT)
$(call gb_ExternalProject_use_external_project,libxml2,icu)

$(eval $(call gb_ExternalProject_use_nmake,libxml2,build))

$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_Trace_StartRange,libxml2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		cscript /e:javascript configure.js \
			iconv=no icu=yes sax1=yes $(if $(MSVC_USE_DEBUG_RUNTIME),cruntime=/MDd) \
			$(if $(filter TRUE,$(ENABLE_DBGUTIL)),debug=yes) \
		&& nmake \
	,win32)
	$(call gb_Trace_EndRange,libxml2,EXTERNAL)
else # OS!=WNT
$(call gb_ExternalProject_get_state_target,libxml2,build):
	$(call gb_Trace_StartRange,libxml2,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --disable-ipv6 --without-iconv --without-python --without-zlib --with-sax1 \
			--without-lzma \
			$(if $(debug),--with-run-debug) \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(filter MACOSX,$(OS)),--prefix=/@.__________________________________________________URELIB) \
			LDFLAGS="$(LDFLAGS) $(call gb_ExternalProject_get_link_flags,libxml2) $(if $(SYSBASE),-L$(SYSBASE)/usr/lib)" \
			CFLAGS="$(CFLAGS) \
				$(if $(SYSBASE),-I$(SYSBASE)/usr/include) \
				$(call gb_ExternalProject_get_build_flags,libxml2)" \
			$(if $(filter TRUE,$(DISABLE_DYNLOADING)),--disable-shared,--disable-static) \
		&& $(MAKE) libxml2.la xmllint \
	)
	$(call gb_Trace_EndRange,libxml2,EXTERNAL)
endif

# vim: set noet sw=4 ts=4:
