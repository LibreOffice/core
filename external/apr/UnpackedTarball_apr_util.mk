# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,apr_util))

$(eval $(call gb_UnpackedTarball_set_tarball,apr_util,$(APR_UTIL_TARBALL)))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,apr_util,\
	aprutil.mak \
))

$(eval $(call gb_UnpackedTarball_add_patches,apr_util,\
	external/apr/windows.build.patch.1 \
))

# vim: set noet sw=4 ts=4:
