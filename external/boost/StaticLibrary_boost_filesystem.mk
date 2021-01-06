# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boost_filesystem))

$(eval $(call gb_StaticLibrary_use_unpacked,boost_filesystem,boost))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,boost_filesystem))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boost_filesystem,\
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_use_external,boost_filesystem,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boost_filesystem,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_filesystem,\
	UnpackedTarball/boost/libs/filesystem/src/codecvt_error_category \
	UnpackedTarball/boost/libs/filesystem/src/directory \
	UnpackedTarball/boost/libs/filesystem/src/exception \
	UnpackedTarball/boost/libs/filesystem/src/operations \
	UnpackedTarball/boost/libs/filesystem/src/path \
	UnpackedTarball/boost/libs/filesystem/src/path_traits \
	UnpackedTarball/boost/libs/filesystem/src/portability \
	UnpackedTarball/boost/libs/filesystem/src/unique_path \
	UnpackedTarball/boost/libs/filesystem/src/utf8_codecvt_facet \
	UnpackedTarball/boost/libs/filesystem/src/windows_file_codecvt \
))

# vim: set noet sw=4 ts=4:
