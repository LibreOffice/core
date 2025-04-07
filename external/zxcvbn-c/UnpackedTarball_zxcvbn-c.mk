# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,zxcvbn-c))

$(eval $(call gb_UnpackedTarball_set_tarball,zxcvbn-c,$(ZXCVBN_C_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,zxcvbn-c,2))

# * external/zxcvbn-c/0001-There-is-no-std-basic_string-int.patch.1 sent upstream as
#   <https://github.com/tsyrogit/zxcvbn-c/pull/31> "There is no std::basic_string<int>":
$(eval $(call gb_UnpackedTarball_add_patches,zxcvbn-c,\
	external/zxcvbn-c/zxcvbn-c-2.5-do-not-use-stdafx.patch \
	external/zxcvbn-c/0001-There-is-no-std-basic_string-int.patch.1 \
))

# vim: set noet sw=4 ts=4:
