# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libexttextcat))

$(eval $(call gb_ExternalProject_register_targets,libexttextcat,\
	build \
))

$(call gb_ExternalProject_get_state_target,libexttextcat,build):
	$(call gb_Trace_StartRange,libexttextcat,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --disable-shared --with-pic \
			$(if $(verbose),--disable-silent-rules,--enable-silent-rules) \
			$(if $(ENABLE_WERROR),--enable-werror,--disable-werror) \
			$(gb_CONFIGURE_PLATFORMS) \
		CFLAGS="$(CFLAGS) $(gb_VISIBILITY_FLAGS) $(if $(debug),$(gb_DEBUGINFO_FLAGS)) $(call gb_ExternalProject_get_build_flags,libexttextcat) \
			$(if $(COM_IS_CLANG),-Qunused-arguments)" \
		LDFLAGS="$(call gb_ExternalProject_get_link_flags,libexttextcat)" \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libexttextcat,EXTERNAL)

# vim: set noet sw=4 ts=4:
