#-*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zxing))

$(eval $(call gb_UnpackedTarball_set_tarball,zxing,$(ZXING_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,zxing,1))

$(eval $(call gb_UnpackedTarball_add_patches,zxing, \
	external/zxing/assume.__cpp_lib_string_view.patch.0 \
	external/zxing/android_include.patch.0 \
	external/zxing/missing-include.patch.0 \
	external/zxing/msvc-lambdacapture.patch.0 \
	external/zxing/coroutine.patch.0 \
))

# vim: set noet sw=4 ts=4:
