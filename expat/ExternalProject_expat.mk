# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,expat))

$(eval $(call gb_ExternalProject_use_unpacked,expat,expat))

$(eval $(call gb_ExternalProject_register_targets,expat,\
	configure \
))

$(call gb_ExternalProject_get_state_target,expat,configure) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
	&& touch $@

# vim: set noet sw=4 ts=4:
