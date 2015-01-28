# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,udunits2))

$(eval $(call gb_ExternalProject_use_autoconf,udunits2,configure))

$(eval $(call gb_ExternalProject_use_autoconf,udunits2,build))

$(eval $(call gb_ExternalProject_register_targets,udunits2,\
	configure \
	build \
))

$(call gb_ExternalProject_get_state_target,udunits2,build) : $(call gb_ExternalProject_get_state_target,udunits2,configure)
	+$(call gb_ExternalProject_run,build,\
		$(MAKE) \
	)

$(call gb_ExternalProject_get_state_target,udunits2,configure) :
	$(call gb_ExternalProject_run,configure,\
		autoreconf -i && \
		MAKE=$(MAKE) ./configure \
			--build=$(if $(filter WNT,$(OS)),i686-pc-cygwin,$(BUILD_PLATFORM)) \
	)

# vim: set noet sw=4 ts=4:
