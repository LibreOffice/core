# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libffi))

$(eval $(call gb_ExternalProject_register_targets,libffi,\
	build \
))

# set prefix so that it ends up in libffi.pc so that pkg-config in python3 works

$(call gb_ExternalProject_get_state_target,libffi,build):
	$(call gb_Trace_StartRange,libffi,EXTERNAL)
	$(call gb_ExternalProject_run,build,\
		./configure \
			--enable-option-checking=fatal \
			$(if $(CROSS_COMPILING),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM)) \
			--enable-static \
			--disable-shared \
			--with-pic \
			--enable-portable-binary \
			CC="$(CC) $(if $(filter LINUX,$(OS)),-fvisibility=hidden)" \
			--prefix=$(call gb_UnpackedTarball_get_dir,libffi)/$(HOST_PLATFORM) \
			--disable-docs \
		&& $(MAKE) \
	)
	$(call gb_Trace_EndRange,libffi,EXTERNAL)

# vim: set noet sw=4 ts=4:
