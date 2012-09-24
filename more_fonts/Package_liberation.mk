# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,font_liberation_inc,$(call gb_UnpackedTarball_get_dir,font_liberation)))

$(eval $(call gb_Package_use_unpacked,font_liberation_inc,font_liberation))

$(eval $(call gb_Package_add_files,font_liberation_inc,pck,\
	LiberationMono-Bold.ttf \
	LiberationMono-BoldItalic.ttf \
	LiberationMono-Italic.ttf \
	LiberationMono-Regular.ttf \
	LiberationSans-Bold.ttf \
	LiberationSans-BoldItalic.ttf \
	LiberationSans-Italic.ttf \
	LiberationSans-Regular.ttf \
	LiberationSerif-Bold.ttf \
	LiberationSerif-BoldItalic.ttf \
	LiberationSerif-Italic.ttf \
	LiberationSerif-Regular.ttf \
))

$(eval $(call gb_Package_add_file,font_liberation_inc,pck/License_liberation.txt,LICENSE))

# vim: set noet sw=4 ts=4:
