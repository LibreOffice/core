# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libnumbertext))

$(eval $(call gb_ExternalProject_use_externals,libnumbertext, \
))

$(eval $(call gb_ExternalProject_register_targets,libnumbertext,\
	build \
))

libnumbertext_CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_CXX11) $(gb_EMSCRIPTEN_CXXFLAGS)

libnumbertext_CPPFLAGS+=$(gb_COMPILERDEFS_STDLIB_DEBUG) $(gb_EMSCRIPTEN_CPPFLAGS)

$(call gb_ExternalProject_get_state_target,libnumbertext,build):
	$(call gb_Trace_StartRange,libnumbertext,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		LIBS="$(gb_STDLIBS) $(LIBS)" \
		$(SHELL) $(gb_RUN_CONFIGURE) ./configure --disable-shared --with-pic \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(ENABLE_WERROR),--enable-werror,--disable-werror) \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(libnumbertext_CPPFLAGS),CPPFLAGS='$(libnumbertext_CPPFLAGS)') \
			CXXFLAGS="$(libnumbertext_CXXFLAGS) \
				$(call gb_ExternalProject_get_build_flags,libnumbertext) \
				$(gb_VISIBILITY_FLAGS) $(gb_VISIBILITY_FLAGS_CXX)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libnumbertext)" \
		&& cd src && $(MAKE) \
	)
	$(call gb_Trace_EndRange,libnumbertext,EXTERNAL)

# vim: set noet sw=4 ts=4:
