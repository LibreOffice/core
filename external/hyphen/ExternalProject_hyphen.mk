# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,hyphen))

ifneq ($(ENABLE_WASM_STRIP_HUNSPELL),TRUE)
$(eval $(call gb_ExternalProject_use_external,hyphen,hunspell))
endif

$(eval $(call gb_ExternalProject_register_targets,hyphen,\
	build \
))

$(call gb_ExternalProject_get_state_target,hyphen,build):
	$(call gb_Trace_StartRange,hyphen,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		$(gb_RUN_CONFIGURE) ./configure --disable-shared \
			$(if $(filter-out iOS,$(OS)),--with-pic) \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) gio_can_sniff=no) \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,hyphen,EXTERNAL)

# vim: set noet sw=4 ts=4:
