# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,wpg))

$(eval $(call gb_Library_use_unpacked,wpg,libwpg))

$(eval $(call gb_Library_use_externals,wpg,\
	boost_headers \
	revenge \
	wpd \
))

$(eval $(call gb_Library_set_warnings_disabled,wpg))

$(eval $(call gb_Library_set_include,wpg,\
    -I$(gb_UnpackedTarball_workdir)/libwpg/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,wpg,\
	-DBOOST_ALL_NO_LIB \
	-DDLL_EXPORT \
	-DLIBWPG_BUILD \
	-DNDEBUG \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,wpg,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,wpg,\
	UnpackedTarball/libwpg/src/lib/WPG1Parser \
	UnpackedTarball/libwpg/src/lib/WPG2Parser \
	UnpackedTarball/libwpg/src/lib/WPGBitmap \
	UnpackedTarball/libwpg/src/lib/WPGColor \
	UnpackedTarball/libwpg/src/lib/WPGDashArray \
	UnpackedTarball/libwpg/src/lib/WPGHeader \
	UnpackedTarball/libwpg/src/lib/WPGTextDataHandler \
	UnpackedTarball/libwpg/src/lib/WPGXParser \
	UnpackedTarball/libwpg/src/lib/WPGraphics \
	UnpackedTarball/libwpg/src/lib/libwpg_utils \
))

# vim: set noet sw=4 ts=4:
