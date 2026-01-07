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
	circle.png \
	circle-green.png \
	triangle.png \
	triangle-green.png \
	pin.png \
	pin-green.png \
	flag.png \
	flag-green.png \
	stars-empty.png \
	redo.png \
	cross.png \
	check.png \
	cross.png \
	circle-blue.png \
	circle-yellow.png \
	triangle-blue.png \
	triangle-yellow.png \
	pin-blue.png \
	pin-yellow.png \
	flag-blue.png \
	flag-yellow.png \
	undo.png \
	stars.png \
	info.png \
	warning.png \
))

# vim: set noet sw=4 ts=4:
