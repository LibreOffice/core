 -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,hunspell))

$(eval $(call gb_UnpackedTarball_set_tarball,hunspell,$(HUNSPELL_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,hunspell,\
	external/hunspell/hunspell-solaris.patch \
	external/hunspell/hunspell-windows.patch \
))

ifeq ($(COM),MSC)
$(eval $(call gb_UnpackedTarball_set_post_action,hunspell,\
	touch src/hunspell/config.h \
))
endif

# vim: set noet sw=4 ts=4:
