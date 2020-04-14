# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,extras_gallmytheme,$(SRCDIR)/extras/source/gallery/gallery_mytheme))

$(eval $(call gb_Package_add_files,extras_gallmytheme,$(LIBO_SHARE_PRESETS_FOLDER)/gallery,\
	sg30.sdv \
	sg30.thm \
	arrows.sdg \
	arrows.sdv \
	arrows.thm \
	bpmn.sdg \
	bpmn.sdv \
	bpmn.thm \
	flowchart.sdg \
	flowchart.sdv \
	flowchart.thm \
	icons.sdg \
	icons.sdv \
	icons.thm \
	shapes.sdg \
	shapes.sdv \
	shapes.thm \
	network.sdg \
	network.sdv \
	network.thm \
	diagrams.sdg \
	diagrams.sdv \
	diagrams.thm \
))
