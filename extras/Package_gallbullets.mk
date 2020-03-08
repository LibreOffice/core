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
	circle-gray.svg \
	circle-green.svg \
	circle-yellow.svg \
	circle-red.svg \
	triangle-blue.svg \
	triangle-green.svg \
	undo.svg \
	redo.svg \
	info.svg \
	check.svg \
	cross.svg \
	warning.svg \
	folder.svg \
	mail.svg \
	mail-send.svg \
	tag.svg \
	flower.svg \
	cloud.svg \
	stars.svg \
	stars-empty.svg \
	world.svg \
	radiobutton.svg \
	checkbox.svg \
	progressbar.svg \
))

# vim: set noet sw=4 ts=4:
