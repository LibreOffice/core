# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,breakpad))

$(eval $(call gb_ExternalProject_register_targets,breakpad,\
	build \
))


ifeq ($(COM),MSC)

else # !ifeq($(COM),MSC)

$(call gb_ExternalProject_get_state_target,breakpad,build) :
	$(call gb_ExternalProject_run,build,\
		./configure \
		&& $(MAKE) \
	)

endif

# vim: set noet sw=4 ts=4:
