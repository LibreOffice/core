# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,afdko))

$(eval $(call gb_UnpackedTarball_set_tarball,afdko,$(AFDKO_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_pre_action,afdko,\
    unzip -q -d a4 -o $(gb_UnpackedTarget_TARFILE_LOCATION)/$(ANTLR4CPPRUNTIME_TARBALL) \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,afdko,1))

# mergeFonts_crash.patch upstream attempt as:
# https://github.com/adobe-type-tools/afdko/pull/1806

$(eval $(call gb_UnpackedTarball_add_patches,afdko, \
    external/afdko/extern_tx.patch \
    external/afdko/extern_tx_shared.patch \
    external/afdko/extern_mergefonts.patch \
    external/afdko/extern_makeotf.patch \
    external/afdko/warnings.patch \
    external/afdko/antlr4-chrono.patch \
    external/afdko/mergeFonts_crash.patch \
))

# vim: set noet sw=4 ts=4:
