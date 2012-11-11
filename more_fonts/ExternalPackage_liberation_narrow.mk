# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,font_liberation_narrow_inc,font_liberation_narrow))

$(eval $(call gb_ExternalPackage_add_unpacked_files,font_liberation_narrow_inc,pck,\
	LiberationSansNarrow-Bold.ttf \
	LiberationSansNarrow-BoldItalic.ttf \
	LiberationSansNarrow-Italic.ttf \
	LiberationSansNarrow-Regular.ttf \
))

$(eval $(call gb_ExternalPackage_add_unpacked_file,font_liberation_narrow_inc,pck/License_liberation_narrow.txt,License.txt))

# vim: set noet sw=4 ts=4:
