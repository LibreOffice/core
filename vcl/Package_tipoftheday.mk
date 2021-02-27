# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,tipoftheday_images,$(SRCDIR)/extras/source/tipoftheday))

$(eval $(call gb_Package_add_files_with_dir,tipoftheday_images,$(LIBO_SHARE_FOLDER)/tipoftheday,\
	tipoftheday.png \
	tipoftheday_w.png \
	tipoftheday_c.png \
	tipoftheday_i.png \
	tipoftheday_d.png \
	toolbarmode.png \
	marchingants.gif \
	printnote.png \
	formdocuments.png \
	masterdocument.png \
	statusbar.png \
	expand_formula_bar.png \
	fraction.png \
	hybrid_pdf.png \
	icon_sets.png \
	remove_hyperlink.png \
	sum_sheets.png \
))

# vim: set noet sw=4 ts=4:
