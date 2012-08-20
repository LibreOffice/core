# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriestemplatescommonlabels,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/common/labels))

$(eval $(call gb_Zip_add_dependencies,accessoriestemplatescommonlabels,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriestemplatescommonlabels,\
	24_port_patch_text.otg \
	arrows-black-base.ott \
	WL-OL1000.ott \
	WL-OL100.ott \
	WL-OL1025.ott \
	WL-OL1050.ott \
	WL-OL1075.ott \
	WL-OL1100.ott \
	WL-OL1125.ott \
	WL-OL1150.ott \
	WL-OL1200.ott \
	WL-OL125.ott \
	WL-OL150.ott \
	WL-OL175.ott \
	WL-OL200.ott \
	WL-OL225.ott \
	WL-OL250.ott \
	WL-OL25.ott \
	WL-OL275.ott \
	WL-OL325.ott \
	WL-OL350.ott \
	WL-OL375.ott \
	WL-OL400.ott \
	WL-OL425.ott \
	WL-OL450.ott \
	WL-OL475.ott \
	WL-OL5100.ott \
	WL-OL525.ott \
	WL-OL5275.ott \
	WL-OL5375.ott \
	WL-OL550.ott \
	WL-OL5575.ott \
	WL-OL5600.ott \
	WL-OL5625.ott \
	WL-OL575.ott \
	WL-OL6000.ott \
	WL-OL600.ott \
	WL-OL6025.ott \
	WL-OL6050.ott \
	WL-OL6075.ott \
	WL-OL625.ott \
	WL-OL675.ott \
	WL-OL725.ott \
	WL-OL750.ott \
	WL-OL75.ott \
	WL-OL775.ott \
	WL-OL800.ott \
	WL-OL850.ott \
	WL-OL875.ott \
	WL-OL900.ott \
	WL-OL975.ott \
))

# vim: set noet sw=4 ts=4:
