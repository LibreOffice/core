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
	check.svg \
	circle-blue.svg \
	circle-green.svg \
	circle-yellow.svg \
	circle.svg \
	cross-red.svg \
	cross.svg \
	flag-blue.svg \
	flag-green.svg \
	flag-yellow.svg \
	flag.svg \
	info.svg \
	pin-blue.svg \
	pin-green.svg \
	pin-yellow.svg \
	pin.svg \
	redo.svg \
	stars-empty.svg \
	stars.svg \
	triangle-blue.svg \
	triangle-green.svg \
	triangle-yellow.svg \
	triangle.svg \
	undo.svg \
	warning.svg \
))

# vim: set noet sw=4 ts=4:
