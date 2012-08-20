# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriesgalleryelementsbullets2,$(call gb_UnpackedTarball_get_dir,gallery-pack)/accessories/Elements/Bullets2))

$(eval $(call gb_Zip_add_dependencies,accessoriesgalleryelementsbullets2,\
	$(call gb_UnpackedTarball_get_target,gallery-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriesgalleryelementsbullets2,\
	amethyst.arrow.png \
	amethyst.asterisk.png \
	amethyst.circle-light.png \
	amethyst.circle.png \
	amethyst.cross1.png \
	amethyst.cross2.png \
	amethyst.dash.png \
	amethyst.diamond-light.png \
	amethyst.diamond.png \
	amethyst.minus.png \
	amethyst.plus.png \
	amethyst.smiley.png \
	amethyst.square-light.png \
	amethyst.square.png \
	amethyst.star.png \
	amethyst.tick.png \
	amethyst.triangle1-light.png \
	amethyst.triangle1.png \
	amethyst.triangle2-light.png \
	amethyst.triangle2.png \
	citrine.arrow.png \
	citrine.asterisk.png \
	citrine.circle-light.png \
	citrine.circle.png \
	citrine.cross1.png \
	citrine.cross2.png \
	citrine.dash.png \
	citrine.diamond-light.png \
	citrine.diamond.png \
	citrine.minus.png \
	citrine.plus.png \
	citrine.smiley.png \
	citrine.square-light.png \
	citrine.square.png \
	citrine.star.png \
	citrine.tick.png \
	citrine.triangle1-light.png \
	citrine.triangle1.png \
	citrine.triangle2-light.png \
	citrine.triangle2.png \
	emerald.arrow.png \
	emerald.asterisk.png \
	emerald.circle-light.png \
	emerald.circle.png \
	emerald.cross1.png \
	emerald.cross2.png \
	emerald.dash.png \
	emerald.diamond-light.png \
	emerald.diamond.png \
	emerald.minus.png \
	emerald.plus.png \
	emerald.smiley.png \
	emerald.square-light.png \
	emerald.square.png \
	emerald.star.png \
	emerald.tick.png \
	emerald.triangle1-light.png \
	emerald.triangle1.png \
	emerald.triangle2-light.png \
	emerald.triangle2.png \
	glass.arrow.png \
	glass.asterisk.png \
	glass.circle-light.png \
	glass.circle.png \
	glass.cross1.png \
	glass.cross2.png \
	glass.dash.png \
	glass.diamond-light.png \
	glass.diamond.png \
	glass.minus.png \
	glass.plus.png \
	glass.smiley.png \
	glass.square-light.png \
	glass.square.png \
	glass.star.png \
	glass.tick.png \
	glass.triangle1-light.png \
	glass.triangle1.png \
	glass.triangle2-light.png \
	glass.triangle2.png \
	mono.arrow.png \
	mono.asterisk.png \
	mono.circle-light.png \
	mono.circle.png \
	mono.circle-semi1.png \
	mono.circle-semi2.png \
	mono.cross1.png \
	mono.cross2.png \
	mono.dash.png \
	mono.diamond-light.png \
	mono.diamond.png \
	mono.diamond-semi1.png \
	mono.diamond-semi2.png \
	mono.minus.png \
	mono.plus.png \
	mono.smiley.png \
	mono.square-light.png \
	mono.square.png \
	mono.square-semi1.png \
	mono.square-semi2.png \
	mono.star.png \
	mono.tick.png \
	mono.triangle1-light.png \
	mono.triangle1.png \
	mono.triangle1-semi.png \
	mono.triangle2-light.png \
	mono.triangle2.png \
	mono.triangle2-semi.png \
	mono.triangle3-light.png \
	mono.triangle3.png \
	mono.triangle3-semi.png \
	mono.triangle4-light.png \
	mono.triangle4.png \
	mono.triangle4-semi.png \
	obsidian.arrow.png \
	obsidian.asterisk.png \
	obsidian.circle-light.png \
	obsidian.circle.png \
	obsidian.cross1.png \
	obsidian.cross2.png \
	obsidian.dash.png \
	obsidian.diamond-light.png \
	obsidian.diamond.png \
	obsidian.minus.png \
	obsidian.plus.png \
	obsidian.smiley.png \
	obsidian.square-light.png \
	obsidian.square.png \
	obsidian.star.png \
	obsidian.tick.png \
	obsidian.triangle1-light.png \
	obsidian.triangle1.png \
	obsidian.triangle2-light.png \
	obsidian.triangle2.png \
	ruby.arrow.png \
	ruby.asterisk.png \
	ruby.circle-light.png \
	ruby.circle.png \
	ruby.cross1.png \
	ruby.cross2.png \
	ruby.dash.png \
	ruby.diamond-light.png \
	ruby.diamond.png \
	ruby.minus.png \
	ruby.plus.png \
	ruby.smiley.png \
	ruby.square-light.png \
	ruby.square.png \
	ruby.star.png \
	ruby.tick.png \
	ruby.triangle1-light.png \
	ruby.triangle1.png \
	ruby.triangle2-light.png \
	ruby.triangle2.png \
	sapphire.arrow.png \
	sapphire.asterisk.png \
	sapphire.circle-light.png \
	sapphire.circle.png \
	sapphire.cross1.png \
	sapphire.cross2.png \
	sapphire.dash.png \
	sapphire.diamond-light.png \
	sapphire.diamond.png \
	sapphire.minus.png \
	sapphire.plus.png \
	sapphire.smiley.png \
	sapphire.square-light.png \
	sapphire.square.png \
	sapphire.star.png \
	sapphire.tick.png \
	sapphire.triangle1-light.png \
	sapphire.triangle1.png \
	sapphire.triangle2-light.png \
	sapphire.triangle2.png \
))

# vim: set noet sw=4 ts=4:
