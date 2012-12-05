# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallcomputers,$(SRCDIR)/extras/source/gallery/computers))

$(eval $(call gb_Package_set_outdir,extras_gallcomputers,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallcomputers,share/gallery/computers,\
	Computer-Cloud.png \
	Computer-Desktop.png \
	Computer-Laptop-Black.png \
	Computer-Laptop-Silver.png \
	Database-Add.png \
	Database-Delete.png \
	Database-Download.png \
	Database.png \
	Folder01-Blue.png \
	Folder02-Green.png \
	Folder03-Manilla.png \
	Folder04-Yellow.png \
	Folder05-OpenBlue.png \
	Folder06-OpenGreen.png \
	Folder07-OpenManilla.png \
	Folder08-OpenYellow.png \
	Server.png \
	WirelessAccessPoint.png \
))

# vim: set noet sw=4 ts=4:
