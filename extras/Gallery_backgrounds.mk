# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,backgrounds,extras/source/gallery/backgrounds))

$(eval $(call gb_Gallery_add_files,backgrounds,$(LIBO_SHARE_FOLDER)/gallery/backgrounds,\
	extras/source/gallery/backgrounds/brick-wall.png \
	extras/source/gallery/backgrounds/cardboard.jpg \
	extras/source/gallery/backgrounds/circuit-board.jpg \
	extras/source/gallery/backgrounds/cloud.jpg \
	extras/source/gallery/backgrounds/coffee.jpg \
	extras/source/gallery/backgrounds/color-stripes.png \
	extras/source/gallery/backgrounds/concrete.jpg \
	extras/source/gallery/backgrounds/crumpled-paper.jpg \
	extras/source/gallery/backgrounds/fence.jpg \
	extras/source/gallery/backgrounds/floral.png \
	extras/source/gallery/backgrounds/fuzzy-darkgrey.jpg \
	extras/source/gallery/backgrounds/fuzzy-grey.jpg \
	extras/source/gallery/backgrounds/fuzzy-lightgrey.jpg \
	extras/source/gallery/backgrounds/giraffe.png \
	extras/source/gallery/backgrounds/graph-paper.png \
	extras/source/gallery/backgrounds/ice-light.jpg \
	extras/source/gallery/backgrounds/invoiced-paper.jpg \
	extras/source/gallery/backgrounds/leaf.jpg \
	extras/source/gallery/backgrounds/maple-leaves.jpg \
	extras/source/gallery/backgrounds/marble.jpg \
	extras/source/gallery/backgrounds/painted-white.jpg \
	extras/source/gallery/backgrounds/parchment-paper.jpg \
	extras/source/gallery/backgrounds/painted-wood.jpg \
	extras/source/gallery/backgrounds/pebble-light.jpg \
	extras/source/gallery/backgrounds/pool.jpg \
	extras/source/gallery/backgrounds/rock-wall.jpg \
	extras/source/gallery/backgrounds/sand-light.jpg \
	extras/source/gallery/backgrounds/sand.jpg \
	extras/source/gallery/backgrounds/sky.jpg \
	extras/source/gallery/backgrounds/space.png \
	extras/source/gallery/backgrounds/stone-gray.jpg \
	extras/source/gallery/backgrounds/stone-wall.jpg \
	extras/source/gallery/backgrounds/stone.jpg \
	extras/source/gallery/backgrounds/stones.jpg \
	extras/source/gallery/backgrounds/studio.jpg \
	extras/source/gallery/backgrounds/surface-black.jpg \
	extras/source/gallery/backgrounds/surface.jpg \
	extras/source/gallery/backgrounds/texture-paper.jpg \
	extras/source/gallery/backgrounds/tiger.jpg \
	extras/source/gallery/backgrounds/tiles.jpg \
	extras/source/gallery/backgrounds/white-diffusion.jpg \
	extras/source/gallery/backgrounds/wood.jpg \
	extras/source/gallery/backgrounds/wooden-board.jpg \
	extras/source/gallery/backgrounds/wooden-fence.jpg \
	extras/source/gallery/backgrounds/zebra.png \
))

# vim: set noet sw=4 ts=4:
