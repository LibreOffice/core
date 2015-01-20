# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,poppler))

$(eval $(call gb_UnpackedTarball_set_tarball,poppler,$(POPPLER_TARBALL),,poppler))

$(eval $(call gb_UnpackedTarball_add_patches,poppler,\
	external/poppler/poppler-notests.patch.1 \
	external/poppler/poppler-snprintf.patch.1 \
	external/poppler/poppler-mac-fake.patch.1 \
	external/poppler/poppler-nochecknew.patch.1 \
	external/poppler/poppler-vs2013.patch.1 \
	external/poppler/poppler-0.26.4-std-max.patch.1 \
	external/poppler/poppler-new-drawchar.patch.1 \
))

# vim: set noet sw=4 ts=4:
