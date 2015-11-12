# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,exttextcat))

$(eval $(call gb_ExternalProject_register_targets,exttextcat,\
	build \
))

$(call gb_ExternalProject_get_state_target,exttextcat,build):
	$(call gb_ExternalProject_run,build,\
		./configure --disable-shared --with-pic \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_DEBUG_CFLAGS) $(gb_COMPILEROPTFLAGS) \
			$(if $(COM_IS_CLANG),-Qunused-arguments) \
			$(if $(filter AIX,$(OS)),-D_LINUX_SOURCE_COMPAT)" \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
