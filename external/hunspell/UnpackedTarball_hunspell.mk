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

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,hunspell))

ifeq ($(COM),MSC)
$(eval $(call gb_UnpackedTarball_set_post_action,hunspell,\
	touch src/hunspell/config.h \
))
endif

$(eval $(call gb_UnpackedTarball_set_patchlevel,hunspell,1))

$(eval $(call gb_UnpackedTarball_add_patches,hunspell, \
	external/hunspell/0001-check-len-in-cpdpat_check-like-r1-blen-is-checked-63.patch \
	external/hunspell/0001-improve-630-test-case-from-0m2.427s-0m1.836s-781.patch \
	external/hunspell/0001-improve-630-test-case-from-0m1.836s-0m1.223s-785.patch \
))

# vim: set noet sw=4 ts=4:
