# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,libwpd))

$(eval $(call gb_ExternalProject_use_unpacked,libwpd,wpd))

$(eval $(call gb_ExternalProject_register_targets,libwpd,\
	build \
))

ifeq ($(OS)$(COM),WNTMSC)

$(call gb_ExternalProject_get_state_target,libwpd,build) :
	cd $(EXTERNAL_WORKDIR)/build/win32 \
	&& $(COMPATH)/vcpackages/vcbuild.exe libwpd.vcproj "Release|Win32" \
	&& touch $@

else

$(call gb_ExternalProject_get_state_target,libwpd,build) :
	cd $(EXTERNAL_WORKDIR) \
	&& ./configure \
		--with-pic \
		--enable-static \
		--disable-shared \
		--without-stream \
		--without-docs \
		--disable-debug \
	&& $(GNUMAKE) \
	&& touch $@

endif

# vim: set noet sw=4 ts=4:
