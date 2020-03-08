# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,bullets,extras/source/gallery/bullets))

$(eval $(call gb_Gallery_add_files,bullets,$(LIBO_SHARE_FOLDER)/gallery/bullets,\
	extras/source/gallery/bullets/circle-gray.svg \
	extras/source/gallery/bullets/circle-green.svg \
	extras/source/gallery/bullets/circle-yellow.svg \
	extras/source/gallery/bullets/circle-red.svg \
	extras/source/gallery/bullets/triangle-blue.svg \
	extras/source/gallery/bullets/triangle-green.svg \
	extras/source/gallery/bullets/undo.svg \
	extras/source/gallery/bullets/redo.svg \
	extras/source/gallery/bullets/info.svg \
	extras/source/gallery/bullets/check.svg \
	extras/source/gallery/bullets/cross.svg \
	extras/source/gallery/bullets/warning.svg \
	extras/source/gallery/bullets/folder.svg \
	extras/source/gallery/bullets/flag-green.svg \
	extras/source/gallery/bullets/flag-red.svg \
	extras/source/gallery/bullets/tag.svg \
	extras/source/gallery/bullets/flower.svg \
	extras/source/gallery/bullets/ball.svg \
	extras/source/gallery/bullets/stars.svg \
	extras/source/gallery/bullets/stars-empty.svg \
	extras/source/gallery/bullets/world.svg \
	extras/source/gallery/bullets/book.svg \
	extras/source/gallery/bullets/checkbox.svg \
	extras/source/gallery/bullets/ok.svg \
))

# vim: set noet sw=4 ts=4:
