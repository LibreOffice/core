# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,shlxthandler_common))

$(eval $(call gb_StaticLibrary_set_include,shlxthandler_common,\
	-I$(SRCDIR)/shell/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,shlxthandler_common,\
	-DISOLATION_AWARE_ENABLED \
	-DUNICODE \
	-DWIN32_LEAN_AND_MEAN \
	-D_NTSDK \
	-D_UNICODE \
))
$(eval $(call gb_StaticLibrary_add_defs,shlxthandler_common,\
	-U_WIN32_IE \
	-D_WIN32_IE=0x501 \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0501 \
))

$(eval $(call gb_StaticLibrary_use_externals,shlxthandler_common,\
	expat \
	zlib \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,shlxthandler_common,\
    shell/source/win32/ooofilereader/autostyletag \
    shell/source/win32/ooofilereader/basereader \
    shell/source/win32/ooofilereader/contentreader \
    shell/source/win32/ooofilereader/keywordstag \
    shell/source/win32/ooofilereader/metainforeader \
    shell/source/win32/ooofilereader/simpletag \
    shell/source/win32/shlxthandler/ooofilt/stream_helper \
    shell/source/win32/shlxthandler/util/fileextensions \
    shell/source/win32/shlxthandler/util/iso8601_converter \
    shell/source/win32/shlxthandler/util/registry \
    shell/source/win32/shlxthandler/util/utilities \
    shell/source/win32/zipfile/zipexcptn \
    shell/source/win32/zipfile/zipfile \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
