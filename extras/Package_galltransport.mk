# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_galltransport,$(SRCDIR)/extras/source/gallery/transportation))

$(eval $(call gb_Package_set_outdir,extras_galltransport,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_galltransport,share/gallery/transport,\
	Airplane-Blue.png \
	Bicycle-Blue.png \
	Boat.png \
	Bus.png \
	Canoe-Blue.png \
	Car-Red.png \
	Helicopter-Blue.png \
	Motorcycle-Red.png \
	Pedestrian-Blue.png \
	PersonalTransporter-Green.png \
	Sailboat-Red.png \
	Scooter-Orange.png \
	Train-Red.png \
	Truck-Blue.png \
))

# vim: set noet sw=4 ts=4:
