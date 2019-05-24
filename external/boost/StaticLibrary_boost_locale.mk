# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boost_locale))

$(eval $(call gb_StaticLibrary_use_unpacked,boost_locale,boost))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,boost_locale))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boost_locale,\
	-DBOOST_ALL_NO_LIB -DBOOST_LOCALE_NO_WINAPI_BACKEND -DBOOST_LOCALE_NO_POSIX_BACKEND -DBOOST_USE_WINDOWS_H \
))

# Needed when building against MSVC resp. libc++ in C++17 mode, as Boost 1.65.1
# workdir/UnpackedTarball/boost/boost/locale/generator.hpp contains "std::auto_ptr<data> d;":
$(eval $(call gb_StaticLibrary_add_defs,boost_locale, \
    -D_HAS_AUTO_PTR_ETC=1 \
    -D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR \
))

$(eval $(call gb_StaticLibrary_use_external,boost_locale,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boost_locale,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_locale,\
	UnpackedTarball/boost/libs/locale/src/encoding/codepage \
	UnpackedTarball/boost/libs/locale/src/shared/date_time \
	UnpackedTarball/boost/libs/locale/src/shared/format \
	UnpackedTarball/boost/libs/locale/src/shared/formatting \
	UnpackedTarball/boost/libs/locale/src/shared/generator \
	UnpackedTarball/boost/libs/locale/src/shared/ids \
	UnpackedTarball/boost/libs/locale/src/shared/localization_backend \
	UnpackedTarball/boost/libs/locale/src/shared/message \
	UnpackedTarball/boost/libs/locale/src/shared/mo_lambda \
	UnpackedTarball/boost/libs/locale/src/std/codecvt \
	UnpackedTarball/boost/libs/locale/src/std/collate \
	UnpackedTarball/boost/libs/locale/src/std/converter \
	UnpackedTarball/boost/libs/locale/src/std/numeric \
	UnpackedTarball/boost/libs/locale/src/std/std_backend \
	UnpackedTarball/boost/libs/locale/src/util/codecvt_converter \
	UnpackedTarball/boost/libs/locale/src/util/default_locale \
	UnpackedTarball/boost/libs/locale/src/util/gregorian \
	UnpackedTarball/boost/libs/locale/src/util/info \
	UnpackedTarball/boost/libs/locale/src/util/locale_data \
))

ifeq ($(OS),WNT)

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_locale,\
	UnpackedTarball/boost/libs/locale/src/win32/collate \
	UnpackedTarball/boost/libs/locale/src/win32/converter \
	UnpackedTarball/boost/libs/locale/src/win32/lcid \
	UnpackedTarball/boost/libs/locale/src/win32/numeric \
	UnpackedTarball/boost/libs/locale/src/win32/win_backend \
))

endif

# vim: set noet sw=4 ts=4:
