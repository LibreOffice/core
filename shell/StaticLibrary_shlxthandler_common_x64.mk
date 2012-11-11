# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
	expat_utf16_x64 \
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
