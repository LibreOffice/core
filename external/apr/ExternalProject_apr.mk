# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,apr))

$(eval $(call gb_ExternalProject_register_targets,apr,\
	build \
))

$(eval $(call gb_ExternalProject_use_nmake,apr,build))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,apr,build):
	$(call gb_ExternalProject_run,build,\
		nmake -nologo -f apr.mak \
			CFG="apr - Win32 $(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)" \
			RECURSE=0 \
	)

else
$(call gb_ExternalProject_get_state_target,apr,build):
	+$(call gb_ExternalProject_run,build,\
		 $(if $(ENABLE_MACOSX_SANDBOX),ac_cv_func_fdatasync=no) \
		./configure \
			--enable-static --disable-shared \
			--with-pic \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
		&& $(MAKE) libapr-1.la \
	)

endif

# vim: set noet sw=4 ts=4:
