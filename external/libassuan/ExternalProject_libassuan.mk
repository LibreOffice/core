# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libassuan))

$(eval $(call gb_ExternalProject_register_targets,libassuan,\
	build \
))

$(eval $(call gb_ExternalProject_use_autoconf,libassuan,build))

$(call gb_ExternalProject_get_state_target,libassuan,build):
	$(call gb_ExternalProject_run,build,\
		autoreconf \
		&& ./configure \
		   GPG_ERROR_CFLAGS="$(GPG_ERROR_CFLAGS)" \
		   GPG_ERROR_LIBS="$(GPG_ERROR_LIBS)" \
	  && $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
