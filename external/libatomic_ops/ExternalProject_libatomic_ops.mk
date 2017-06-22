# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libatomic_ops))

$(eval $(call gb_ExternalProject_use_autoconf,libatomic_ops,build))

$(eval $(call gb_ExternalProject_register_targets,libatomic_ops,\
	build \
))

$(call gb_ExternalProject_get_state_target,libatomic_ops,build) :
	$(call gb_ExternalProject_run,build,\
		$(if $(filter TRUE,$(DISABLE_DYNLOADING)),CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))" CXXFLAGS="$(CXXFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_VISIBILITY_FLAGS_CXX) $(if $(ENABLE_OPTIMIZED),$(gb_COMPILEROPTFLAGS),$(gb_COMPILERNOOPTFLAGS))") \
		./configure \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
		&& $(MAKE) \
	)

# vim: set noet sw=4 ts=4:
