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

libnumbertext_CXXFLAGS=$(CXXFLAGS) $(CXXFLAGS_CXX11)

ifneq (,$(filter ANDROID DRAGONFLY FREEBSD iOS LINUX NETBSD OPENBSD,$(OS)))
ifneq (,$(gb_ENABLE_DBGUTIL))
libnumbertext_CPPFLAGS+=-D_GLIBCXX_DEBUG
endif
endif

$(call gb_ExternalProject_get_state_target,libnumbertext,build):
	$(call gb_ExternalProject_run,build,\
		LIBS="$(gb_STDLIBS) $(LIBS)" \
		$(SHELL) ./configure --disable-shared --with-pic \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(ENABLE_WERROR),--enable-werror,--disable-werror) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
			$(if $(filter AIX,$(OS)),CFLAGS="-D_LINUX_SOURCE_COMPAT") \
			$(if $(libnumbertext_CPPFLAGS),CPPFLAGS='$(libnumbertext_CPPFLAGS)') \
			CXXFLAGS="$(libnumbertext_CXXFLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS)) $(if $(debug),$(gb_DEBUGINFO_FLAGS)) $(gb_VISIBILITY_FLAGS) $(gb_VISIBILITY_FLAGS_CXX)" \
		&& cd src && $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
