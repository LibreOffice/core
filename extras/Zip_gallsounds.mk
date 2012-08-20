# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,gallsounds,$(SRCDIR)/extras/source/gallery/sounds))

$(eval $(call gb_Zip_add_files,gallsounds,\
	apert2.wav \
	apert.wav \
	applause.wav \
	beam2.wav \
	beam.wav \
	cow.wav \
	curve.wav \
	drama.wav \
	explos.wav \
	falling.wav \
	glasses.wav \
	gong.wav \
	horse.wav \
	kling.wav \
	kongas.wav \
	laser.wav \
	left.wav \
	nature1.wav \
	nature2.wav \
	ok.wav \
	pluck.wav \
	roll.wav \
	romans.wav \
	soft.wav \
	space2.wav \
	space3.wav \
	space.wav \
	sparcle.wav \
	strom.wav \
	theetone.wav \
	top.wav \
	train.wav \
	untie.wav \
	ups.wav \
	wallewal.wav \
))

# vim: set noet sw=4 ts=4:
