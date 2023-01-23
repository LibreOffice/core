# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,hunspell))

$(eval $(call gb_ExternalProject_register_targets,hunspell,\
	build \
))

hunspell_CPPCLAGS=$(CPPFLAGS)

hunspell_CPPFLAGS+=$(gb_COMPILERDEFS_STDLIB_DEBUG)

hunspell_CXXFLAGS:=$(CXXFLAGS) $(gb_LTOFLAGS) \
       $(gb_EMSCRIPTEN_CPPFLAGS) \
       $(call gb_ExternalProject_get_build_flags,hunspell)

hunspell_LDFLAGS:=$(gb_LTOFLAGS) $(call gb_ExternalProject_get_link_flags,hunspell)

$(call gb_ExternalProject_get_state_target,hunspell,build):
	$(call gb_Trace_StartRange,hunspell,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --disable-shared --disable-nls --with-pic \
			$(gb_CONFIGURE_PLATFORMS) \
			$(if $(hunspell_CPPFLAGS),CPPFLAGS='$(hunspell_CPPFLAGS)') \
			$(if $(hunspell_CXXFLAGS),CXXFLAGS='$(hunspell_CXXFLAGS)') \
			$(if $(hunspell_LDFLAGS),LDFLAGS='$(hunspell_LDFLAGS)') \
		&& cd src/hunspell && $(MAKE) \
	)
	$(call gb_Trace_EndRange,hunspell,EXTERNAL)

# vim: set noet sw=4 ts=4:
