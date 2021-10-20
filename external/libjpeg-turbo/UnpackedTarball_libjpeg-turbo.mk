# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libjpeg-turbo))

$(eval $(call gb_UnpackedTarball_set_tarball,libjpeg-turbo,$(LIBJPEG_TURBO_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libjpeg-turbo,0))

# jpeg-turbo.default_config.0 generated via
# cmake ../libjpeg-turbo-2.1.1 -DENABLE_STATIC:BOOL=ON -DENABLE_SHARED:BOOL=NO -DWITH_JAVA:BOOL=OFF -DWITH_TURBOJPEG:BOOL=OFF -DWITH_SIMD:BOOL=OFF

$(eval $(call gb_UnpackedTarball_add_patches,libjpeg-turbo,\
	external/libjpeg-turbo/jpeg-turbo.default_config.0 \
))

# vim: set noet sw=4 ts=4:
