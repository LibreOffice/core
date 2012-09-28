# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,font_libertineg_inc,$(call gb_UnpackedTarball_get_dir,font_libertineg)))

$(eval $(call gb_Package_use_unpacked,font_libertineg_inc,font_libertineg))

$(eval $(call gb_Package_add_files,font_libertineg_inc,pck,\
	LinBiolinum_RB_G.ttf \
	LinBiolinum_RI_G.ttf \
	LinBiolinum_R_G.ttf \
	LinLibertine_DR_G.ttf \
	LinLibertine_RBI_G.ttf \
	LinLibertine_RB_G.ttf \
	LinLibertine_RI_G.ttf \
	LinLibertine_RZI_G.ttf \
	LinLibertine_RZ_G.ttf \
	LinLibertine_R_G.ttf \
))

$(eval $(call gb_Package_add_file,font_libertineg_inc,pck/License_libertineg.txt,OFL.txt))

