# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,mythes))

$(eval $(call gb_ExternalProject_use_unpacked,mythes,mythes))

$(eval $(call gb_ExternalProject_use_package,mythes,hunspell))

$(eval $(call gb_ExternalProject_register_targets,mythes,\
	build \
))

$(call gb_ExternalProject_get_state_target,mythes,build):
	cd $(EXTERNAL_WORKDIR) \
	&& LIBS="$(gb_STDLIBS) $(LIBS)" ./configure --disable-shared --with-pic \
	$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) gio_can_sniff=no) \
	HUNSPELL_CFLAGS=" $(HUNSPELL_CFLAGS)" \
	HUNSPELL_LIBS=" $(HUNSPELL_LIBS)" \
	$(if $(filter C52U,$(COM)$(CPU)),CFLAGS="-m64") \
	&& $(MAKE) \
	&& touch $@

# vim: set noet sw=4 ts=4:
