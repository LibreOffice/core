# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,environment,extras/source/gallery/environment))

$(eval $(call gb_Gallery_add_files,environment,$(LIBO_SHARE_FOLDER)/gallery/environment,\
	extras/source/gallery/environment/DrippingFaucet.png \
	extras/source/gallery/environment/Earth.png \
	extras/source/gallery/environment/EndangeredAnimals1.png \
	extras/source/gallery/environment/EndangeredAnimals2.png \
	extras/source/gallery/environment/GreenCar.png \
	extras/source/gallery/environment/GreenFactory.png \
	extras/source/gallery/environment/GreenHouse.png \
	extras/source/gallery/environment/Leaf1.png \
	extras/source/gallery/environment/Leaf2.png \
	extras/source/gallery/environment/LightBulb-Flourescent-Off.png \
	extras/source/gallery/environment/LightBulb-Flourescent-On.png \
	extras/source/gallery/environment/LightBulb-Standard-Off1.png \
	extras/source/gallery/environment/LightBulb-Standard-Off2.png \
	extras/source/gallery/environment/LightBulb-Standard-On.png \
	extras/source/gallery/environment/Pollution-Car.png \
	extras/source/gallery/environment/Pollution-Factory.png \
	extras/source/gallery/environment/Raindrop.png \
	extras/source/gallery/environment/RecycleBin.png \
	extras/source/gallery/environment/RecycleSymbol.png \
	extras/source/gallery/environment/RenewableEnergy-Solar.png \
	extras/source/gallery/environment/RenewableEnergySymbol.png \
	extras/source/gallery/environment/RenewableEnergy-Water.png \
	extras/source/gallery/environment/RenewableEnergy-Wind.png \
	extras/source/gallery/environment/Sun1.png \
	extras/source/gallery/environment/Sun2.png \
))

# vim: set noet sw=4 ts=4:
