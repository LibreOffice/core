# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,computers,extras/source/gallery/computers))

$(eval $(call gb_Gallery_add_files,computers,$(LIBO_SHARE_FOLDER)/gallery/computers,\
    extras/source/gallery/computers/Computer-Cloud.png \
    extras/source/gallery/computers/Computer-Desktop.png \
    extras/source/gallery/computers/Computer-Laptop-Black.png \
    extras/source/gallery/computers/Computer-Laptop-Silver.png \
    extras/source/gallery/computers/Database-Add.png \
    extras/source/gallery/computers/Database-Delete.png \
    extras/source/gallery/computers/Database-Download.png \
    extras/source/gallery/computers/Database.png \
    extras/source/gallery/computers/Folder01-Blue.png \
    extras/source/gallery/computers/Folder02-Green.png \
    extras/source/gallery/computers/Folder03-Manilla.png \
    extras/source/gallery/computers/Folder04-Yellow.png \
    extras/source/gallery/computers/Folder05-OpenBlue.png \
    extras/source/gallery/computers/Folder06-OpenGreen.png \
    extras/source/gallery/computers/Folder07-OpenManilla.png \
    extras/source/gallery/computers/Folder08-OpenYellow.png \
    extras/source/gallery/computers/Server.png \
    extras/source/gallery/computers/WirelessAccessPoint.png \
))

# vim: set noet sw=4 ts=4:
