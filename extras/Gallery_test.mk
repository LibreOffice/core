# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,extras/source/gallery,gallery,Fun Gallery))

$(eval $(call gb_Gallery_add_files,extras/source/gallery, \
	extras/source/gallery/bigapple.gif \
	extras/source/gallery/sky.gif      \
	extras/source/gallery/flower.gif   \
	extras/source/gallery/apples.gif ))

# vim: set noet sw=4 ts=4:
