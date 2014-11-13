# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,jpeg))

$(eval $(call gb_ExternalProject_use_autoconf,jpeg,configure))

$(eval $(call gb_ExternalProject_register_targets,jpeg,\
	configure \
	build \
))

$(call gb_ExternalProject_get_state_target,jpeg,build) : $(call gb_ExternalProject_get_state_target,jpeg,configure)
	+$(call gb_ExternalProject_run,build,\
		$(MAKE) \
	)

$(call gb_ExternalProject_get_state_target,jpeg,configure) :
	$(call gb_ExternalProject_run,configure,\
		MAKE=$(MAKE) ./configure \
			--with-pic \
			--enable-static \
			--disable-shared \
			--with-jpeg8 \
			--without-java \
			--without-turbojpeg \
	)

# vim: set noet sw=4 ts=4:
