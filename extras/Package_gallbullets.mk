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
	circle.svg \
	circle-green.svg \
	triangle.svg \
	triangle-green.svg \
	pin.svg \
	pin-green.svg \
	flag.svg \
	flag-green.svg \
	stars-empty.svg \
	redo.svg \
	cross.svg \
	check.svg \
	cross.svg \
	circle-blue.svg \
	circle-yellow.svg \
	triangle-blue.svg \
	triangle-yellow.svg \
	pin-blue.svg \
	pin-yellow.svg \
	flag-blue.svg \
	flag-yellow.svg \
	undo.svg \
	stars.svg \
	info.svg \
	warning.svg \
))

# vim: set noet sw=4 ts=4:
