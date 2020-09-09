# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,wps))

$(eval $(call gb_Library_use_unpacked,wps,libwps))

$(eval $(call gb_Library_use_externals,wps,\
	revenge \
))

$(eval $(call gb_Library_set_warnings_disabled,wps))

$(eval $(call gb_Library_set_include,wps,\
    -I$(call gb_UnpackedTarball_get_dir,libwps)/inc \
    -I$(call gb_UnpackedTarball_get_dir,libwps)/src/lib \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,wps,external/libwps/inc/pch/precompiled_wps))

$(eval $(call gb_Library_add_defs,wps,\
	-DBUILD_WPS\
	-DDLL_EXPORT \
	-DNDEBUG \
))

$(eval $(call gb_Library_set_generated_cxx_suffix,wps,cpp))

$(eval $(call gb_Library_add_generated_exception_objects,wps,\
	UnpackedTarball/libwps/src/lib/DosWord \
	UnpackedTarball/libwps/src/lib/Lotus \
	UnpackedTarball/libwps/src/lib/LotusChart \
	UnpackedTarball/libwps/src/lib/LotusGraph \
	UnpackedTarball/libwps/src/lib/LotusSpreadsheet \
	UnpackedTarball/libwps/src/lib/LotusStyleManager \
	UnpackedTarball/libwps/src/lib/MSWrite \
	UnpackedTarball/libwps/src/lib/Multiplan \
	UnpackedTarball/libwps/src/lib/PocketWord \
	UnpackedTarball/libwps/src/lib/Quattro \
	UnpackedTarball/libwps/src/lib/Quattro9 \
	UnpackedTarball/libwps/src/lib/Quattro9Graph \
	UnpackedTarball/libwps/src/lib/Quattro9Spreadsheet \
	UnpackedTarball/libwps/src/lib/QuattroDos \
	UnpackedTarball/libwps/src/lib/QuattroDosChart \
	UnpackedTarball/libwps/src/lib/QuattroDosSpreadsheet \
	UnpackedTarball/libwps/src/lib/QuattroFormula \
	UnpackedTarball/libwps/src/lib/QuattroGraph \
	UnpackedTarball/libwps/src/lib/QuattroSpreadsheet \
	UnpackedTarball/libwps/src/lib/WKS4 \
	UnpackedTarball/libwps/src/lib/WKS4Chart \
	UnpackedTarball/libwps/src/lib/WKS4Format \
	UnpackedTarball/libwps/src/lib/WKS4Spreadsheet \
	UnpackedTarball/libwps/src/lib/WKSChart \
	UnpackedTarball/libwps/src/lib/WKSContentListener \
	UnpackedTarball/libwps/src/lib/WKSParser \
	UnpackedTarball/libwps/src/lib/WKSSubDocument \
	UnpackedTarball/libwps/src/lib/WPS4 \
	UnpackedTarball/libwps/src/lib/WPS4Graph \
	UnpackedTarball/libwps/src/lib/WPS4Text \
	UnpackedTarball/libwps/src/lib/WPS8 \
	UnpackedTarball/libwps/src/lib/WPS8Graph \
	UnpackedTarball/libwps/src/lib/WPS8Struct \
	UnpackedTarball/libwps/src/lib/WPS8Table \
	UnpackedTarball/libwps/src/lib/WPS8Text \
	UnpackedTarball/libwps/src/lib/WPS8TextStyle \
	UnpackedTarball/libwps/src/lib/WPSCell \
	UnpackedTarball/libwps/src/lib/WPSContentListener \
	UnpackedTarball/libwps/src/lib/WPSDebug \
	UnpackedTarball/libwps/src/lib/WPSDocument \
	UnpackedTarball/libwps/src/lib/WPSEntry \
	UnpackedTarball/libwps/src/lib/WPSFont \
	UnpackedTarball/libwps/src/lib/WPSGraphicShape \
	UnpackedTarball/libwps/src/lib/WPSGraphicStyle \
	UnpackedTarball/libwps/src/lib/WPSHeader \
	UnpackedTarball/libwps/src/lib/WPSList \
	UnpackedTarball/libwps/src/lib/WPSListener \
	UnpackedTarball/libwps/src/lib/WPSOLE1Parser \
	UnpackedTarball/libwps/src/lib/WPSOLEObject \
	UnpackedTarball/libwps/src/lib/WPSOLEParser \
	UnpackedTarball/libwps/src/lib/WPSOLEStream \
	UnpackedTarball/libwps/src/lib/WPSPageSpan \
	UnpackedTarball/libwps/src/lib/WPSParagraph \
	UnpackedTarball/libwps/src/lib/WPSParser \
	UnpackedTarball/libwps/src/lib/WPSStream \
	UnpackedTarball/libwps/src/lib/WPSStringStream \
	UnpackedTarball/libwps/src/lib/WPSSubDocument \
	UnpackedTarball/libwps/src/lib/WPSTable \
	UnpackedTarball/libwps/src/lib/WPSTextParser \
	UnpackedTarball/libwps/src/lib/WPSTextSubDocument \
	UnpackedTarball/libwps/src/lib/XYWrite \
	UnpackedTarball/libwps/src/lib/libwps_internal \
	UnpackedTarball/libwps/src/lib/libwps_tools_win \
))

# vim: set noet sw=4 ts=4:
