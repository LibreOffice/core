# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,wps))

$(eval $(call gb_UnpackedTarball_set_tarball,wps,$(WPS_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,wps,1))

$(eval $(call gb_UnpackedTarball_add_patches,wps,\
    libwps/libwps.msvc.warnings.patch \
    libwps/libwps.gcc.warnings.patch \
))

$(eval $(call gb_UnpackedTarball_mark_output_files,wps,\
	$(addsuffix .cpp,$(libwps_SOURCEFILES)) \
	src/lib/libwps_internal.h \
	src/lib/libwps_tools_win.h \
	src/lib/WPS4Graph.h \
	src/lib/WPS4.h \
	src/lib/WPS4Text.h \
	src/lib/WPS8.h \
	src/lib/WPSCell.h \
	src/lib/WPSContentListener.h \
	src/lib/WPSDebug.h \
	src/lib/WPSEntry.h \
	src/lib/WPS.h \
	src/lib/WPSHeader.h \
	src/lib/WPSList.h \
	src/lib/WPSOLEParser.h \
	src/lib/WPSOLEStream.h \
	src/lib/WPSPageSpan.h \
	src/lib/WPSParagraph.h \
	src/lib/WPSParser.h \
	src/lib/WPSPosition.h \
	src/lib/WPSSubDocument.h \
	src/lib/WPSTable.h \
))

# vim: set noet sw=4 ts=4:
