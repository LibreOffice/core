# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallsystem,$(SRCDIR)/extras/source/gallery/gallery_system))

$(eval $(call gb_Package_add_files,extras_gallsystem,$(LIBO_SHARE_FOLDER)/gallery,\
	arrows.sdg \
	arrows.sdv \
	arrows.thm \
	bpmn.sdg \
	bpmn.sdv \
	bpmn.thm \
	bullets.sdg \
	bullets.sdv \
	bullets.thm \
	diagrams.sdg \
	diagrams.sdv \
	diagrams.thm \
	flowchart.sdg \
	flowchart.sdv \
	flowchart.thm \
	fontwork.sdg \
	fontwork.sdv \
	fontwork.thm \
	icons.sdg \
	icons.sdv \
	icons.thm \
	network.sdg \
	network.sdv \
	network.thm \
	shapes.sdg \
	shapes.sdv \
	shapes.thm \
	symbolshapes.sdg \
	symbolshapes.sdv \
	symbolshapes.thm \
))

# vim: set noet sw=4 ts=4:
