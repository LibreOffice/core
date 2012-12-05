# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallenvironment,$(SRCDIR)/extras/source/gallery/environment))

$(eval $(call gb_Package_set_outdir,extras_gallenvironment,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallenvironment,share/gallery/environment,\
	DrippingFaucet.png \
	Earth.png \
	EndangeredAnimals1.png \
	EndangeredAnimals2.png \
	GreenCar.png \
	GreenFactory.png \
	GreenHouse.png \
	Leaf1.png \
	Leaf2.png \
	LightBulb-Flourescent-Off.png \
	LightBulb-Flourescent-On.png \
	LightBulb-Standard-Off1.png \
	LightBulb-Standard-Off2.png \
	LightBulb-Standard-On.png \
	Pollution-Car.png \
	Pollution-Factory.png \
	Raindrop.png \
	RecycleBin.png \
	RecycleSymbol.png \
	RenewableEnergy-Solar.png \
	RenewableEnergySymbol.png \
	RenewableEnergy-Water.png \
	RenewableEnergy-Wind.png \
	Sun1.png \
	Sun2.png \
))

# vim: set noet sw=4 ts=4:
