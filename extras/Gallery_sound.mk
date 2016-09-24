# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# this should probably be done with more general rules: but how ?

$(eval $(call gb_Gallery_Gallery,sounds,extras/source/gallery/sounds))

$(eval $(call gb_Gallery_add_files,sounds,$(LIBO_SHARE_FOLDER)/gallery/sounds,\
	extras/source/gallery/sounds/apert2.wav \
	extras/source/gallery/sounds/apert.wav \
	extras/source/gallery/sounds/applause.wav \
	extras/source/gallery/sounds/beam2.wav \
	extras/source/gallery/sounds/beam.wav \
	extras/source/gallery/sounds/cow.wav \
	extras/source/gallery/sounds/curve.wav \
	extras/source/gallery/sounds/drama.wav \
	extras/source/gallery/sounds/explos.wav \
	extras/source/gallery/sounds/falling.wav \
	extras/source/gallery/sounds/glasses.wav \
	extras/source/gallery/sounds/gong.wav \
	extras/source/gallery/sounds/horse.wav \
	extras/source/gallery/sounds/kling.wav \
	extras/source/gallery/sounds/kongas.wav \
	extras/source/gallery/sounds/laser.wav \
	extras/source/gallery/sounds/left.wav \
	extras/source/gallery/sounds/nature1.wav \
	extras/source/gallery/sounds/nature2.wav \
	extras/source/gallery/sounds/ok.wav \
	extras/source/gallery/sounds/pluck.wav \
	extras/source/gallery/sounds/roll.wav \
	extras/source/gallery/sounds/romans.wav \
	extras/source/gallery/sounds/soft.wav \
	extras/source/gallery/sounds/space2.wav \
	extras/source/gallery/sounds/space3.wav \
	extras/source/gallery/sounds/space.wav \
	extras/source/gallery/sounds/sparcle.wav \
	extras/source/gallery/sounds/strom.wav \
	extras/source/gallery/sounds/theetone.wav \
	extras/source/gallery/sounds/top.wav \
	extras/source/gallery/sounds/train.wav \
	extras/source/gallery/sounds/untie.wav \
	extras/source/gallery/sounds/ups.wav \
	extras/source/gallery/sounds/wallewal.wav \
))

# vim: set noet sw=4 ts=4: \
