# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libabw))

$(eval $(call gb_UnpackedTarball_set_tarball,libabw,$(ABW_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,libabw,0))

ifeq ($(COM_IS_CLANG),TRUE)
ifneq ($(filter -fsanitize=%,$(CC)),)
$(eval $(call gb_UnpackedTarball_add_patches,libabw, \
    external/libabw/ubsan-visibility.patch \
))
endif
endif

# vim: set noet sw=4 ts=4:
