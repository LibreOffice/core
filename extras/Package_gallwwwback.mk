# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallwwwback,$(SRCDIR)/extras/source/gallery/www-back))

$(eval $(call gb_Package_add_files,extras_gallwwwback,$(LIBO_SHARE_FOLDER)/gallery/www-back,\
	brick-wall.png \
	cardboard.jpg \
	circuit-board.jpg \
	cloud.jpg \
	coffee.jpg \
	color-stripes.png \
	concrete.jpg \
	crumpled-paper \
	fence.jpg \
	floral.png \
	fuzzy-darkgrey.jpg \
	fuzzy-grey.jpg \
	fuyyz-lightgrey \
	giraffe.png \
	graph-paper.png \
	ice-light.jpg \
	incoiced-paper.jpg \
	leaf.jpg \
	maple-leaves.jpg \
	marble.jpg \
	painted-white.jpg \
	parchment-paper.jpg \
	painted-wood.jpg \
	pebble-light.jpg \
	pool.jpg \
	rock-wall.jpg \
	sand-light.jpg \
	sand.jpg \
	sky.jpg \
	space.png \
	stone-gray.jpg \
	stone-wall.jpg \
	stone.jpg \
	stones.jpg \
	studio.jpg \
	surface-black.jpg \
	surface.jpg \
	texture-paper.jpg \
	tiger.jpg \
	tiles.jpg \
	white-diffusion.jpg \
	wood.jpg \
	wooden-board.jpg \
	wooden-fence.jpg \
	zebra.png \
))

# vim: set noet sw=4 ts=4:
