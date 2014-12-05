# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,jpeg-turbo))

$(eval $(call gb_ExternalProject_use_autoconf,jpeg-turbo,configure))
$(eval $(call gb_ExternalProject_use_autoconf,jpeg-turbo,build))

$(eval $(call gb_ExternalProject_register_targets,jpeg-turbo,\
	configure \
	build \
))

$(call gb_ExternalProject_get_state_target,jpeg-turbo,build) : $(call gb_ExternalProject_get_state_target,jpeg-turbo,configure)
	+$(call gb_ExternalProject_run,build,\
		CPPFLAGS="-I$(call gb_UnpackedTarball_get_dir,clcc)/src $$CPPFLAGS" $(MAKE) \
	)

$(call gb_ExternalProject_get_state_target,jpeg-turbo,configure) :
	$(call gb_ExternalProject_run,configure,\
		MAKE=$(MAKE) CPPFLAGS="-I$(call gb_UnpackedTarball_get_dir,clcc)/src $$CPPFLAGS" ./configure \
			--build=$(if $(filter WNT,$(OS)),i686-pc-cygwin,$(BUILD_PLATFORM)) \
			--with-pic \
			--enable-static \
			--disable-shared \
			--with-jpeg8 \
			--without-java \
			--without-turbojpeg \
			$(if $(filter-out ANDROID IOS,$(OS)),--with-opencl-dec) \
	)

# vim: set noet sw=4 ts=4:
