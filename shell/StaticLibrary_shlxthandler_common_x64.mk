# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,shlxthandler_common_x64))

$(eval $(call gb_StaticLibrary_set_x64,shlxthandler_common_x64,YES))

$(eval $(call gb_StaticLibrary_use_custom_headers,shlxthandler_common_x64,\
	shell/source \
))

$(eval $(call gb_StaticLibrary_set_include,shlxthandler_common_x64,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,shlxthandler_common_x64,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
))
$(eval $(call gb_StaticLibrary_add_defs,shlxthandler_common_x64,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
))

$(eval $(call gb_StaticLibrary_use_externals,shlxthandler_common_x64,\
	expat_x64 \
	zlib_x64 \
))

$(eval $(call gb_StaticLibrary_add_x64_generated_exception_objects,shlxthandler_common_x64,\
    CustomTarget/shell/source/win32/ooofilereader/autostyletag \
    CustomTarget/shell/source/win32/ooofilereader/basereader \
    CustomTarget/shell/source/win32/ooofilereader/contentreader \
    CustomTarget/shell/source/win32/ooofilereader/keywordstag \
    CustomTarget/shell/source/win32/ooofilereader/metainforeader \
    CustomTarget/shell/source/win32/ooofilereader/simpletag \
    CustomTarget/shell/source/win32/shlxthandler/ooofilt/stream_helper \
    CustomTarget/shell/source/win32/shlxthandler/util/fileextensions \
    CustomTarget/shell/source/win32/shlxthandler/util/iso8601_converter \
    CustomTarget/shell/source/win32/shlxthandler/util/registry \
    CustomTarget/shell/source/win32/shlxthandler/util/utilities \
    CustomTarget/shell/source/win32/zipfile/zipexcptn \
    CustomTarget/shell/source/win32/zipfile/zipfile \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
