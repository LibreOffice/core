# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,gallrulers,$(SRCDIR)/extras/source/gallery/rulers))

$(eval $(call gb_Zip_add_files,gallrulers,\
	blkballs.gif \
	blurulr1.gif \
	blurulr2.gif \
	blurulr3.gif \
	blurulr4.gif \
	blurulr5.gif \
	blurulr6.gif \
	gldballs.gif \
	grnballs.gif \
	grnrulr1.gif \
	grnrulr2.gif \
	grnrulr3.gif \
	grnrulr4.gif \
	gryrulr1.gif \
	gryrulr2.gif \
	gryrulr3.gif \
	orgrulr1.gif \
	redrulr1.gif \
	redrulr2.gif \
	redrulr3.gif \
	redrulr4.gif \
	redrulr5.gif \
	striped.gif \
	whtballs.gif \
	ylwrulr1.gif \
))

# vim: set noet sw=4 ts=4:
