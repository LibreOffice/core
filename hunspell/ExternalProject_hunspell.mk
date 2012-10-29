# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,hunspell))

$(eval $(call gb_ExternalProject_use_unpacked,hunspell,hunspell))

$(eval $(call gb_ExternalProject_register_targets,hunspell,\
	build \
))

$(call gb_ExternalProject_get_state_target,hunspell,build):
	cd $(EXTERNAL_WORKDIR) \
	&& LIBS="$(addprefix -l,$(gb_STDLIBS)) $(LIBS)" ./configure --disable-shared --disable-nls --with-pic \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
	$(if $(filter AIX,$(OS)),CFLAGS="-D_LINUX_SOURCE_COMPAT") \
	$(if $(filter C53,$(COM)),CFLAGS="-xc99=none") \
	$(if $(filter-out WNTGCC,$(GUI)$(COM)),,LDFLAGS="-Wl,--enable-runtime-pseudo-reloc-v2") \
	&& $(GNUMAKE) -j$(EXTMAXPROCESS) \
	&& touch $@

# vim: set noet sw=4 ts=4:
