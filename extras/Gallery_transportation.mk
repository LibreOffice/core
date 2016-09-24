# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,transportation,extras/source/gallery/transportation))

$(eval $(call gb_Gallery_add_files,transportation,$(LIBO_SHARE_FOLDER)/gallery/transportation,\
    extras/source/gallery/transportation/Airplane-Blue.png \
    extras/source/gallery/transportation/Bicycle-Blue.png \
    extras/source/gallery/transportation/Boat.png \
    extras/source/gallery/transportation/Bus.png \
    extras/source/gallery/transportation/Canoe-Blue.png \
    extras/source/gallery/transportation/Car-Red.png \
    extras/source/gallery/transportation/Helicopter-Blue.png \
    extras/source/gallery/transportation/Motorcycle-Red.png \
    extras/source/gallery/transportation/Pedestrian-Blue.png \
    extras/source/gallery/transportation/PersonalTransporter-Green.png \
    extras/source/gallery/transportation/Sailboat-Red.png \
    extras/source/gallery/transportation/Scooter-Orange.png \
    extras/source/gallery/transportation/Train-Red.png \
    extras/source/gallery/transportation/Truck-Blue.png \
))

# vim: set noet sw=4 ts=4:
