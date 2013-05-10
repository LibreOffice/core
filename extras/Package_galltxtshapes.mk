# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_galltxtshapes,$(SRCDIR)/extras/source/gallery/txtshapes))

$(eval $(call gb_Package_set_outdir,extras_galltxtshapes,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_galltxtshapes,share/gallery/textshapes,\
	Circle01-DarkBlue.png \
	Circle02-LightBlue.png \
	Circle03-Green.png \
	Circle04-DarkRed.png \
	Circle05-Orange.png \
	Hexagon01-DarkBlue.png \
	Hexagon02-Blue.png \
	Hexagon03-Green.png \
	Hexagon04-DarkRed.png \
	Hexagon05-Orange.png \
	Leaf01-DarkBlue.png \
	Leaf02-LightBlue.png \
	Leaf03-Green.png \
	Leaf04-DarkRed.png \
	Leaf05-Orange.png \
	Paster01-DarkBlue.png \
	Paster02-LightBlue.png \
	Paster03-Green.png \
	Paster04-Red.png \
	Paster05-Orange.png \
	Rectangle01-DarkBlue.png \
	Rectangle02-LightBlue.png \
	Rectangle03-Green.png \
	Rectangle04-DarkRed.png \
	Rectangle05-Orange.png \
	Rectangle06-Striped-Blue.png \
	Rectangle07-Striped-Green.png \
	Rectangle08-Striped-Red.png \
	Rectangle09-Striped-Orange.png \
	Square01-DarkBlue.png \
	Square02-LightBlue.png \
	Square03-Green.png \
	Square04-DarkRed.png \
	Square05-Orange.png \
	Square06-Striped-Blue.png \
	Square07-Striped-Green.png \
	Square08-Striped-Red.png \
	Square09-Striped-Orange.png \
))

# vim: set noet sw=4 ts=4:
