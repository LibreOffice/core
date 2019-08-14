# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Gallery_Gallery,bpmn,extras/source/gallery/bpmn))

$(eval $(call gb_Gallery_add_files,bpmn,$(LIBO_SHARE_FOLDER)/gallery/bpmn,\
	extras/source/gallery/bpmn/bpmn.sdg \
	extras/source/gallery/bpmn/bpmn.sdv \
	extras/source/gallery/bpmn/bpmn.thm \
))

# vim: set noet sw=4 ts=4:
