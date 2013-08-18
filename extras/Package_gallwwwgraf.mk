# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallwwwgraf,$(SRCDIR)/extras/source/gallery/www-graf))

$(eval $(call gb_Package_set_outdir,extras_gallwwwgraf,$(INSTDIR)))

$(eval $(call gb_Package_add_files,extras_gallwwwgraf,$(LIBO_SHARE_FOLDER)/gallery/www-graf,\
	bluat.gif \
	bluback.gif \
	bludisk.gif \
	bludown.gif \
	bluhome.gif \
	bluinfo.gif \
	bluleft.gif \
	blumail.gif \
	bluminus.gif \
	bluplus.gif \
	bluquest.gif \
	bluright.gif \
	bluup.gif \
	gredisk.gif \
	gredown.gif \
	grehome.gif \
	greinfo.gif \
	greleft.gif \
	gremail.gif \
	greminus.gif \
	greplus.gif \
	grequest.gif \
	greright.gif \
	greup.gif \
	grnat.gif \
	grnback.gif \
	grndisk.gif \
	grndown.gif \
	grnexcla.gif \
	grnhome.gif \
	grninfo.gif \
	grnleft.gif \
	grnmail.gif \
	grnminus.gif \
	grnplus.gif \
	grnquest.gif \
	grnright.gif \
	grnup.gif \
	gryat.gif \
	gryback.gif \
	grydisk.gif \
	grydown.gif \
	gryhome.gif \
	gryinfo.gif \
	gryleft.gif \
	grymail.gif \
	gryminus.gif \
	gryplus.gif \
	gryquest.gif \
	gryright.gif \
	gryup.gif \
	men@work.gif \
	orgat.gif \
	orgback.gif \
	orgdisk.gif \
	orgdown.gif \
	orghome.gif \
	orginfo.gif \
	orgleft.gif \
	orgmail.gif \
	orgminus.gif \
	orgplus.gif \
	orgquest.gif \
	orgright.gif \
	orgup.gif \
	redat.gif \
	redback.gif \
	reddisk.gif \
	reddown.gif \
	redhome.gif \
	redinfo.gif \
	redleft.gif \
	redmail.gif \
	redminus.gif \
	redplus.gif \
	redquest.gif \
	redright.gif \
	redup.gif \
	turdown.gif \
	turhome.gif \
	turleft.gif \
	turright.gif \
	turup.gif \
	viohome.gif \
	violeft.gif \
	vioright.gif \
	vioup.gif \
	ylwdown.gif \
	ylwhome.gif \
	ylwleft.gif \
	ylwmail.gif \
	ylwright.gif \
	ylwup.gif \
))

# vim: set noet sw=4 ts=4:
