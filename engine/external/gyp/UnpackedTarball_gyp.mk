# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,gyp))

$(eval $(call gb_UnpackedTarball_set_tarball,gyp,$(GYP_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,gyp,0))

# external/gyp/gyp-python-deprecated.patch upstream at
#  <https://github.com/nodejs/gyp-next/commit/428343ee58c05e7b817d3603688fe7cdb7a10846> "fix: avoid
#  deprecated os.path.commonprefix() in RelativePath (#365)":
$(eval $(call gb_UnpackedTarball_add_patches,gyp,\
	external/gyp/gyp-concurrent-futures.patch.1 \
	external/gyp/gyp-msvc-env-compiler-cache.patch.1 \
	external/gyp/gyp-python-deprecated.patch \
))

# vim: set noet sw=4 ts=4:
