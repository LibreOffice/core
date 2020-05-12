# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallbullets,$(SRCDIR)/extras/source/gallery/bullets))

$(eval $(call gb_Package_add_files,extras_gallbullets,$(LIBO_SHARE_FOLDER)/gallery/bullets,\
	circle-gray.png \
	circle-green.png \
	circle-yellow.png \
	circle-red.png \
	triangle-blue.png \
	triangle-green.png \
	undo.png \
	redo.png \
	info.png \
	check.png \
	cross.png \
	warning.png \
	folder.png \
	tag.png \
	flower.png \
	stars.png \
	stars-empty.png \
	world.png \
	checkbox.png \
))

# vim: set noet sw=4 ts=4:
