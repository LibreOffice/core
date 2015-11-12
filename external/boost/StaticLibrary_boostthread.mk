# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boostthread))

$(eval $(call gb_StaticLibrary_use_unpacked,boostthread,boost))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,boostthread))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boostthread,\
	-DBOOST_ALL_NO_LIB \
))

# Lots of the declarations in boost duplicated from windows.h when
# BOOST_USE_WINDOWS_H is not defined (which appears to be the normal case) cause
# warnings or even errors with Clang when windows.h is included too (e.g.,
# "conflicting types for 'FileTimeToLocalFileTime'" between
# C:/PROGRA~2/WI3CF2~1/8.1/include/um/fileapi.h and
# workdir/UnpackedTarball/boost/boost/date_time/filetime_functions.hpp), for
# which it appears easies to just define BOOST_USE_WINDOWS_H; also
# -Winvalid-constexpr (e.g., reported from the constexpr definition of lowest()
# in workdir/UnpackedTarball/boost/boost/chrono/duration.hpp, which uses
# std::numeric_limits<>::max() from MSVC's standard library, where nothing is
# marked constexpr) is apparently reported as an error by default:
ifeq ($(OS),WNT)
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_StaticLibrary_add_defs,boostthread, \
    -DBOOST_USE_WINDOWS_H \
    -Wno-error=invalid-constexpr \
))
endif
endif

$(eval $(call gb_StaticLibrary_use_external,boostthread,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boostthread,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boostthread,\
	UnpackedTarball/boost/libs/thread/src/win32/thread \
	UnpackedTarball/boost/libs/thread/src/win32/tss_dll \
	UnpackedTarball/boost/libs/thread/src/win32/tss_pe \
))

# vim: set noet sw=4 ts=4:
