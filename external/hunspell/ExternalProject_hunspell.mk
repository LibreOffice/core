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

ifneq (,$(filter ANDROID DRAGONFLY FREEBSD iOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
hunspell_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

$(call gb_ExternalProject_get_state_target,hunspell,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-shared --disable-nls --with-pic \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
			$(if $(filter AIX,$(OS)),CFLAGS="-D_LINUX_SOURCE_COMPAT") \
			$(if $(hunspell_CPPFLAGS),CPPFLAGS='$(hunspell_CPPFLAGS)') \
			CXXFLAGS="$(CXXFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS))" \
		&& cd src/hunspell && $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
