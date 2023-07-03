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
	$(call gb_Trace_StartRange,libatomic_ops,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(if $(filter TRUE, \
			$(DISABLE_DYNLOADING)) \
			, \
			CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(call gb_ExternalProject_get_build_flags,libatomic_ops)" \
			CXXFLAGS="$(CXXFLAGS) $(gb_VISIBILITY_FLAGS) $(gb_VISIBILITY_FLAGS_CXX) $(call gb_ExternalProject_get_build_flags,libatomic_ops)" \
			LDFLAGS="$(call gb_ExternalProject_get_link_flags,libatomic_ops)") \
		$(gb_RUN_CONFIGURE) ./configure \
			--disable-gpl \
			$(gb_CONFIGURE_PLATFORMS) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libatomic_ops,EXTERNAL)

# vim: set noet sw=4 ts=4:
