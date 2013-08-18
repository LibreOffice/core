# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallwwwback,$(SRCDIR)/extras/source/gallery/www-back))

$(eval $(call gb_Package_set_outdir,extras_gallwwwback,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallwwwback,$(LIBO_SHARE_FOLDER)/gallery/www-back,\
	aqua.jpg \
	bathroom.jpg \
	blocks.jpg \
	blow_green.jpg \
	blueblop.jpg \
	bulging.jpg \
	canvas_blue.jpg \
	cheese.jpg \
	chocolate.jpg \
	citrus.jpg \
	confetti.jpg \
	daisy.jpg \
	fluffy-grey.jpg \
	fluffy.jpg \
	fuzzy-blue.jpg \
	fuzzy-darkgrey.jpg \
	fuzzy-grey.jpg \
	fuzzy-lightgrey.jpg \
	fuzzy_light.jpg \
	gregre.gif \
	grey.gif \
	grypaws.gif \
	ice-blue.jpg \
	ice-light.jpg \
	imitation_leather.jpg \
	interstices.jpg \
	jeansblk.jpg \
	jeans.jpg \
	lawn-artificial.jpg \
	lawn.jpg \
	lightblue-wet.jpg \
	linen-fine.jpg \
	lino-green.jpg \
	liquid-blue.jpg \
	marble_dark.jpg \
	marble.jpg \
	mazes.jpg \
	mint.gif \
	notes.gif \
	pattern.jpg \
	pebble-light.jpg \
	pink.gif \
	pool.jpg \
	popcorn.jpg \
	purple.jpg \
	reddark.jpg \
	rings-green.jpg \
	rings-orange.jpg \
	roses.jpg \
	sand.jpg \
	sand-light.jpg \
	sky.jpg \
	soft-structure_grey.jpg \
	space.jpg \
	stone-dark.jpg \
	stone.jpg \
	structure_darkgreen.gif \
	structure_green.jpg \
	structure.jpg \
	wall-grey.jpg \
	wet-turquoise.jpg \
	wood.jpg \
))

# vim: set noet sw=4 ts=4:
