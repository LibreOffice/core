# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,serf))

$(eval $(call gb_ExternalProject_use_externals,serf,\
	apr \
))

$(eval $(call gb_ExternalProject_register_targets,serf,\
	build \
))

$(eval $(call gb_ExternalProject_use_nmake,serf,build))

ifeq ($(COM),MSC)
$(call gb_ExternalProject_get_state_target,serf,build):
	$(call gb_ExternalProject_run,build,\
		APR_SRC="..\apr" \
		APRUTIL_SRC="..\apr_util" \
		OPENSSL_SRC="..\openssl" \
		ZLIB_SRC="..\zlib" \
		nmake -nologo -f serf.mak \
			$(if $(MSVC_USE_DEBUG_RUNTIME),DEBUG_BUILD=T Debug,Release)/serf-1.lib \
	)

else
# serf is using SERF_LIBS variable, so pass it empty
$(call gb_ExternalProject_get_state_target,serf,build):
	+$(call gb_ExternalProject_run,build,\
		./configure SERF_LIBS= \
			--enable-option-checking=fatal \
			$(if $(filter YES,$(CROSS_COMPILING)),--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM))\
			$(if $(SYSTEM_APR),,--with-apr=$(call gb_UnpackedTarball_get_dir,apr)/apr-1-config) \
			$(if $(SYSTEM_APR),,--with-apr-util=$(call gb_UnpackedTarball_get_dir,apr_util)/apu-1-config) \
			$(if $(SYSTEM_OPENSSL),,--with-openssl=$(call gb_UnpackedTarball_get_dir,openssl)) \
		&& $(MAKE) libserf-1.la \
	)

endif

# vim: set noet sw=4 ts=4:
