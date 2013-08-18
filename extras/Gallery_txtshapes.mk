# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,txtshapes,extras/source/gallery/txtshapes))

$(eval $(call gb_Gallery_add_files,txtshapes,$(LIBO_SHARE_FOLDER)/gallery/txtshapes,\
	extras/source/gallery/txtshapes/Circle01-DarkBlue.svg \
	extras/source/gallery/txtshapes/Circle02-LightBlue.svg \
	extras/source/gallery/txtshapes/Circle03-Green.svg \
	extras/source/gallery/txtshapes/Circle04-DarkRed.svg \
	extras/source/gallery/txtshapes/Circle05-Orange.svg \
	extras/source/gallery/txtshapes/Hexagon01-DarkBlue.svg \
	extras/source/gallery/txtshapes/Hexagon02-Blue.svg \
	extras/source/gallery/txtshapes/Hexagon03-Green.svg \
	extras/source/gallery/txtshapes/Hexagon04-DarkRed.svg \
	extras/source/gallery/txtshapes/Hexagon05-Orange.svg \
	extras/source/gallery/txtshapes/Leaf01-DarkBlue.svg \
	extras/source/gallery/txtshapes/Leaf02-LightBlue.svg \
	extras/source/gallery/txtshapes/Leaf03-Green.svg \
	extras/source/gallery/txtshapes/Leaf04-DarkRed.svg \
	extras/source/gallery/txtshapes/Leaf05-Orange.svg \
	extras/source/gallery/txtshapes/Paster01-DarkBlue.svg \
	extras/source/gallery/txtshapes/Paster02-LightBlue.svg \
	extras/source/gallery/txtshapes/Paster03-Green.svg \
	extras/source/gallery/txtshapes/Paster04-Red.svg \
	extras/source/gallery/txtshapes/Paster05-Orange.svg \
	extras/source/gallery/txtshapes/Rectangle01-DarkBlue.svg \
	extras/source/gallery/txtshapes/Rectangle02-LightBlue.svg \
	extras/source/gallery/txtshapes/Rectangle03-Green.svg \
	extras/source/gallery/txtshapes/Rectangle04-DarkRed.svg \
	extras/source/gallery/txtshapes/Rectangle05-Orange.svg \
	extras/source/gallery/txtshapes/Rectangle06-Striped-Blue.svg \
	extras/source/gallery/txtshapes/Rectangle07-Striped-Green.svg \
	extras/source/gallery/txtshapes/Rectangle08-Striped-Red.svg \
	extras/source/gallery/txtshapes/Rectangle09-Striped-Orange.svg \
	extras/source/gallery/txtshapes/Square01-DarkBlue.svg \
	extras/source/gallery/txtshapes/Square02-LightBlue.svg \
	extras/source/gallery/txtshapes/Square03-Green.svg \
	extras/source/gallery/txtshapes/Square04-DarkRed.svg \
	extras/source/gallery/txtshapes/Square05-Orange.svg \
	extras/source/gallery/txtshapes/Square06-Striped-Blue.svg \
	extras/source/gallery/txtshapes/Square07-Striped-Green.svg \
	extras/source/gallery/txtshapes/Square08-Striped-Red.svg \
	extras/source/gallery/txtshapes/Square09-Striped-Orange.svg \
))

# vim: set noet sw=4 ts=4:
