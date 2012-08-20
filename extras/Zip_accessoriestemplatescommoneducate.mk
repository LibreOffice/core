# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriestemplatescommoneducate,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/common/educate))

$(eval $(call gb_Zip_add_dependencies,accessoriestemplatescommoneducate,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriestemplatescommoneducate,\
	A4-chequered.otg \
	A4-lin_2mm.otg \
	A4-lin_3mm.otg \
	A4-lined.otg \
	A4-linlog.otg \
	A4-lin.otg \
	A4-loglin.otg \
	A4-loglog.otg \
	A4-millimeter_black.ott \
	A4-millimeter_orange.ott \
	A4-millimeter_red.ott \
	A4-rhomb.otg \
	Black_lines_1.otg \
	Black_lines_2.otg \
	Black_lines_3.otg \
	soccer-pitch.otg \
	soccer-practice-template.ott \
))

# vim: set noet sw=4 ts=4:
