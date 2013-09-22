# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallhtmlexpo,$(SRCDIR)/extras/source/gallery/htmlexpo))

$(eval $(call gb_Package_set_outdir,extras_gallhtmlexpo,$(INSTROOT)))

$(eval $(call gb_Package_add_files,extras_gallhtmlexpo,$(LIBO_SHARE_FOLDER)/gallery/htmlexpo,\
	bludown.gif \
	blufirs_.gif \
	blufirs.gif \
	blulast_.gif \
	blulast.gif \
	blunav.gif \
	blunext_.gif \
	blunext.gif \
	bluprev_.gif \
	bluprev.gif \
	blutext.gif \
	bluup.gif \
	cubdown.gif \
	cubfirs_.gif \
	cubfirs.gif \
	cublast_.gif \
	cublast.gif \
	cubnav.gif \
	cubnext_.gif \
	cubnext.gif \
	cubprev_.gif \
	cubprev.gif \
	cubtext.gif \
	cubup.gif \
	gredown.gif \
	grefirs_.gif \
	grefirs.gif \
	grelast_.gif \
	grelast.gif \
	grenav.gif \
	grenext_.gif \
	grenext.gif \
	greprev_.gif \
	greprev.gif \
	gretext.gif \
	greup.gif \
	simdown.gif \
	simfirs_.gif \
	simfirs.gif \
	simlast_.gif \
	simlast.gif \
	simnav.gif \
	simnext_.gif \
	simnext.gif \
	simprev_.gif \
	simprev.gif \
	simtext.gif \
	simup.gif \
))

# vim: set noet sw=4 ts=4:
