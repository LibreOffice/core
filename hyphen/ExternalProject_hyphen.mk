# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,hyphen))

$(eval $(call gb_ExternalProject_use_external,hyphen,hunspell))

$(eval $(call gb_ExternalProject_register_targets,hyphen,\
	build \
))

$(call gb_ExternalProject_get_state_target,hyphen,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-shared \
			$(if $(filter-out IOS,$(OS)),--with-pic) \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) gio_can_sniff=no) \
			$(if $(filter C52U,$(COM)$(CPU)),CFLAGS="-m64") \
		&& $(MAKE) \
			$(if $(filter YESYES,$(SYSTEM_HYPH)$(WITH_MYSPELL_DICTS)),hyph_en_US.dic) \
	)

# vim: set noet sw=4 ts=4:
