# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,bzip2))

$(eval $(call gb_ExternalProject_register_targets,bzip2,\
	build \
))

$(eval $(call gb_ExternalProject_use_nmake,bzip2,build))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,bzip2,build):
	$(call gb_ExternalProject_run,build,\
		nmake -nologo -f makefile.msc \
	)
else
$(call gb_ExternalProject_get_state_target,bzip2,build):
	$(call gb_ExternalProject_run,build,\
		$(MAKE) \
	)
endif

# vim: set noet sw=4 ts=4:
