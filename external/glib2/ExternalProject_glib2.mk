# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,glib2))

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalProject_register_targets,glib2,\
	build \
))

$(call gb_ExternalProject_get_state_target,glib2,build):
	$(call gb_ExternalProject_run,build,\
		unset debug && nmake -f makefile.msc \
	,win32)
endif

# vim: set noet sw=4 ts=4:
