# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,wpslib))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,wpslib))

$(eval $(call gb_StaticLibrary_use_unpacked,wpslib,wps))

$(eval $(call gb_StaticLibrary_use_package,wpslib,libwps_inc))

$(eval $(call gb_StaticLibrary_use_externals,wpslib,\
	wpd \
))

$(eval $(call gb_StaticLibrary_set_cxx_suffix,wpslib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,wpslib,\
	UnpackedTarball/wps/src/lib/libwps_internal \
	UnpackedTarball/wps/src/lib/libwps_tools_win \
	UnpackedTarball/wps/src/lib/WPS4 \
	UnpackedTarball/wps/src/lib/WPS4Graph \
	UnpackedTarball/wps/src/lib/WPS4Text \
	UnpackedTarball/wps/src/lib/WPS8 \
	UnpackedTarball/wps/src/lib/WPSCell \
	UnpackedTarball/wps/src/lib/WPSContentListener \
	UnpackedTarball/wps/src/lib/WPSDebug \
	UnpackedTarball/wps/src/lib/WPSDocument \
	UnpackedTarball/wps/src/lib/WPSHeader \
	UnpackedTarball/wps/src/lib/WPSList \
	UnpackedTarball/wps/src/lib/WPSOLEParser \
	UnpackedTarball/wps/src/lib/WPSOLEStream \
	UnpackedTarball/wps/src/lib/WPSPageSpan \
	UnpackedTarball/wps/src/lib/WPSParagraph \
	UnpackedTarball/wps/src/lib/WPSParser \
	UnpackedTarball/wps/src/lib/WPSSubDocument \
	UnpackedTarball/wps/src/lib/WPSTable \
))

# vim: set noet sw=4 ts=4:
