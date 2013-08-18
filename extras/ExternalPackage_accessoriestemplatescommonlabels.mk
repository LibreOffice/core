# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriestemplatescommonlabels,templates-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriestemplatescommonlabels,$(INSTDIR)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriestemplatescommonlabels,$(LIBO_SHARE_FOLDER)/template/common/labels,\
	templates/common/labels/24_port_patch_text.otg \
	templates/common/labels/arrows-black-base.ott \
	templates/common/labels/WL-OL1000.ott \
	templates/common/labels/WL-OL100.ott \
	templates/common/labels/WL-OL1025.ott \
	templates/common/labels/WL-OL1050.ott \
	templates/common/labels/WL-OL1075.ott \
	templates/common/labels/WL-OL1100.ott \
	templates/common/labels/WL-OL1125.ott \
	templates/common/labels/WL-OL1150.ott \
	templates/common/labels/WL-OL1200.ott \
	templates/common/labels/WL-OL125.ott \
	templates/common/labels/WL-OL150.ott \
	templates/common/labels/WL-OL175.ott \
	templates/common/labels/WL-OL200.ott \
	templates/common/labels/WL-OL225.ott \
	templates/common/labels/WL-OL250.ott \
	templates/common/labels/WL-OL25.ott \
	templates/common/labels/WL-OL275.ott \
	templates/common/labels/WL-OL325.ott \
	templates/common/labels/WL-OL350.ott \
	templates/common/labels/WL-OL375.ott \
	templates/common/labels/WL-OL400.ott \
	templates/common/labels/WL-OL425.ott \
	templates/common/labels/WL-OL450.ott \
	templates/common/labels/WL-OL475.ott \
	templates/common/labels/WL-OL5100.ott \
	templates/common/labels/WL-OL525.ott \
	templates/common/labels/WL-OL5275.ott \
	templates/common/labels/WL-OL5375.ott \
	templates/common/labels/WL-OL550.ott \
	templates/common/labels/WL-OL5575.ott \
	templates/common/labels/WL-OL5600.ott \
	templates/common/labels/WL-OL5625.ott \
	templates/common/labels/WL-OL575.ott \
	templates/common/labels/WL-OL6000.ott \
	templates/common/labels/WL-OL600.ott \
	templates/common/labels/WL-OL6025.ott \
	templates/common/labels/WL-OL6050.ott \
	templates/common/labels/WL-OL6075.ott \
	templates/common/labels/WL-OL625.ott \
	templates/common/labels/WL-OL675.ott \
	templates/common/labels/WL-OL725.ott \
	templates/common/labels/WL-OL750.ott \
	templates/common/labels/WL-OL75.ott \
	templates/common/labels/WL-OL775.ott \
	templates/common/labels/WL-OL800.ott \
	templates/common/labels/WL-OL850.ott \
	templates/common/labels/WL-OL875.ott \
	templates/common/labels/WL-OL900.ott \
	templates/common/labels/WL-OL975.ott \
))

# vim: set noet sw=4 ts=4:
