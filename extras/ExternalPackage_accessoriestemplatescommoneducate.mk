# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,extras_accessoriestemplatescommoneducate,templates-pack))

$(eval $(call gb_ExternalPackage_set_outdir,extras_accessoriestemplatescommoneducate,$(gb_INSTROOT)))

$(eval $(call gb_ExternalPackage_add_unpacked_files,extras_accessoriestemplatescommoneducate,$(LIBO_SHARE_FOLDER)/template/common/educate,\
	templates/common/educate/A4-chequered.otg \
	templates/common/educate/A4-lin_2mm.otg \
	templates/common/educate/A4-lin_3mm.otg \
	templates/common/educate/A4-lined.otg \
	templates/common/educate/A4-linlog.otg \
	templates/common/educate/A4-lin.otg \
	templates/common/educate/A4-loglin.otg \
	templates/common/educate/A4-loglog.otg \
	templates/common/educate/A4-millimeter_black.ott \
	templates/common/educate/A4-millimeter_orange.ott \
	templates/common/educate/A4-millimeter_red.ott \
	templates/common/educate/A4-rhomb.otg \
	templates/common/educate/Black_lines_1.otg \
	templates/common/educate/Black_lines_2.otg \
	templates/common/educate/Black_lines_3.otg \
	templates/common/educate/soccer-pitch.otg \
	templates/common/educate/soccer-practice-template.ott \
))

# vim: set noet sw=4 ts=4:
