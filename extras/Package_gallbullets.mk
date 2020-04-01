# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallbullets,$(SRCDIR)/extras/source/gallery/bullets))

$(eval $(call gb_Package_add_files,extras_gallbullets,$(LIBO_SHARE_FOLDER)/gallery/bullets,\
	Bullet01-Circle-DarkRed.svg \
	Bullet02-Circle-Blue.svg \
	Bullet03-Circle-Green.svg \
	Bullet04-Square-Black.svg \
	Bullet05-Square-Orange.svg \
	Bullet06-Square-Purple.svg \
	Bullet07-Diamond-Blue.svg \
	Bullet08-Diamond-LightBlue.svg \
	Bullet09-Diamond-Red.svg \
	Bullet10-Star-Yellow.svg \
	Bullet11-Star-Blue.svg \
	Bullet12-Triangle-Blue.svg \
	Bullet13-Triangle-DarkGreen.svg \
	Bullet14-Arrow-Red.svg \
	Bullet15-Arrow-Blue.svg \
	Bullet16-Box-Blue.svg \
	Bullet17-Box-Red.svg \
	Bullet18-Asterisk-LightBlue.svg \
	Bullet19-Leaves-Red.svg \
	Bullet20-Target-Blue.svg \
	Bullet21-Arrow-Blue.svg \
	Bullet22-Arrow-DarkBlue.svg \
	Bullet23-Arrow-Brown.svg \
	Bullet24-Flag-Red.svg \
	Bullet25-Flag-Green.svg \
	Bullet26-X-Red.svg \
	Bullet27-X-Black.svg \
	Bullet28-Checkmark-Green.svg \
	Bullet29-Checkmark-Blue.svg \
	Bullet30-Square-DarkRed.svg \
	blkpearl.gif \
	bluarrow.gif \
	bluball.gif \
	bludiamd.gif \
	bluered.gif \
	blusqare.gif \
	blustar.gif \
	coffee_1.gif \
	coffee_2.gif \
	coffee_3.gif \
	coffee_4.gif \
	coffee_5.gif \
	con-blue.gif \
	con-cyan.gif \
	con-green.gif \
	con-lilac.gif \
	con-oran.gif \
	con-pink.gif \
	con-red.gif \
	con-yellow.gif \
	corner_1.gif \
	corner_2.gif \
	corner_3.gif \
	corner_4.gif \
	darkball.gif \
	darkblue.gif \
	gldpearl.gif \
	golfball.gif \
	grnarrow.gif \
	grnball.gif \
	grndiamd.gif \
	grnpearl.gif \
	grnsqare.gif \
	grnstar.gif \
	gryarrow.gif \
	gryball.gif \
	grydiamd.gif \
	grysqare.gif \
	grystar.gif \
	orgarrow.gif \
	orgball.gif \
	orgdiamd.gif \
	orgsqare.gif \
	orgstar.gif \
	pebble_1.gif \
	pebble_2.gif \
	pebble_3.gif \
	poliball.gif \
	popcorn_1.gif \
	popcorn_2.gif \
	rainbow.gif \
	redarrow.gif \
	redball.gif \
	reddiamd.gif \
	redsqare.gif \
	redstar.gif \
	whtpearl.gif \
	ylwarrow.gif \
	ylwball.gif \
	ylwdiamd.gif \
	ylwsqare.gif \
	ylwstar.gif \
))

# vim: set noet sw=4 ts=4:
