# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,libwpg))

$(eval $(call gb_Library_use_unpacked,libwpg,libwpg))

$(eval $(call gb_Library_use_externals,libwpg,\
    boost_headers \
	revenge \
))

$(eval $(call gb_Library_set_warnings_not_errors,libwpg))

$(eval $(call gb_Library_set_include,libwpg,\
    -I$(call gb_UnpackedTarball_get_dir,libwpg)/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,libwpg,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBWPG_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_add_generated_exception_objects,libwpg,\
	UnpackedTarball/libwpg/src/lib/WPG1Parser.cpp \
	UnpackedTarball/libwpg/src/lib/WPG2Parser.cpp \
	UnpackedTarball/libwpg/src/lib/WPGBitmap.cpp \
	UnpackedTarball/libwpg/src/lib/WPGColor.cpp \
	UnpackedTarball/libwpg/src/lib/WPGDashArray.cpp \
	UnpackedTarball/libwpg/src/lib/WPGHeader.cpp \
	UnpackedTarball/libwpg/src/lib/WPGXParser.cpp \
	UnpackedTarball/libwpg/src/lib/WPGraphics.cpp \
))

# vim: set noet sw=4 ts=4:
