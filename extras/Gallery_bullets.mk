# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,bullets,extras/source/gallery/bullets))

$(eval $(call gb_Gallery_add_files,bullets,$(LIBO_SHARE_FOLDER)/gallery/bullets,\
	extras/source/gallery/bullets/Bullet01-Circle-DarkRed.svg \
	extras/source/gallery/bullets/Bullet02-Circle-Blue.svg \
	extras/source/gallery/bullets/Bullet03-Circle-Green.svg \
	extras/source/gallery/bullets/Bullet04-Square-Black.svg \
	extras/source/gallery/bullets/Bullet05-Square-Orange.svg \
	extras/source/gallery/bullets/Bullet06-Square-Purple.svg \
	extras/source/gallery/bullets/Bullet07-Diamond-Blue.svg \
	extras/source/gallery/bullets/Bullet08-Diamond-LightBlue.svg \
	extras/source/gallery/bullets/Bullet09-Diamond-Red.svg \
	extras/source/gallery/bullets/Bullet10-Star-Yellow.svg \
	extras/source/gallery/bullets/Bullet11-Star-Blue.svg \
	extras/source/gallery/bullets/Bullet12-Triangle-Blue.svg \
	extras/source/gallery/bullets/Bullet13-Triangle-DarkGreen.svg \
	extras/source/gallery/bullets/Bullet14-Arrow-Red.svg \
	extras/source/gallery/bullets/Bullet15-Arrow-Blue.svg \
	extras/source/gallery/bullets/Bullet16-Box-Blue.svg \
	extras/source/gallery/bullets/Bullet17-Box-Red.svg \
	extras/source/gallery/bullets/Bullet18-Asterisk-LightBlue.svg \
	extras/source/gallery/bullets/Bullet19-Leaves-Red.svg \
	extras/source/gallery/bullets/Bullet20-Target-Blue.svg \
	extras/source/gallery/bullets/Bullet21-Arrow-Blue.svg \
	extras/source/gallery/bullets/Bullet22-Arrow-DarkBlue.svg \
	extras/source/gallery/bullets/Bullet23-Arrow-Brown.svg \
	extras/source/gallery/bullets/Bullet24-Flag-Red.svg \
	extras/source/gallery/bullets/Bullet25-Flag-Green.svg \
	extras/source/gallery/bullets/Bullet26-X-Red.svg \
	extras/source/gallery/bullets/Bullet27-X-Black.svg \
	extras/source/gallery/bullets/Bullet28-Checkmark-Green.svg \
	extras/source/gallery/bullets/Bullet29-Checkmark-Blue.svg \
	extras/source/gallery/bullets/Bullet30-Square-DarkRed.svg \
))

# vim: set noet sw=4 ts=4:
