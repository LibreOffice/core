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

$(eval $(call gb_StaticLibrary_set_include,boost_locale, \
    -I$(gb_UnpackedTarball_workdir)/boost/libs/locale/src \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_use_external,boost_locale,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boost_locale,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_locale,\
	UnpackedTarball/boost/libs/locale/src/boost/locale/encoding/codepage \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/date_time \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/format \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/formatting \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/generator \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/ids \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/localization_backend \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/message \
	UnpackedTarball/boost/libs/locale/src/boost/locale/shared/mo_lambda \
	UnpackedTarball/boost/libs/locale/src/boost/locale/std/codecvt \
	UnpackedTarball/boost/libs/locale/src/boost/locale/std/collate \
	UnpackedTarball/boost/libs/locale/src/boost/locale/std/converter \
	UnpackedTarball/boost/libs/locale/src/boost/locale/std/numeric \
	UnpackedTarball/boost/libs/locale/src/boost/locale/std/std_backend \
	UnpackedTarball/boost/libs/locale/src/boost/locale/util/codecvt_converter \
	UnpackedTarball/boost/libs/locale/src/boost/locale/util/default_locale \
	UnpackedTarball/boost/libs/locale/src/boost/locale/util/encoding \
	UnpackedTarball/boost/libs/locale/src/boost/locale/util/gregorian \
	UnpackedTarball/boost/libs/locale/src/boost/locale/util/info \
	UnpackedTarball/boost/libs/locale/src/boost/locale/util/locale_data \
))

ifeq ($(OS),WNT)

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_locale,\
	UnpackedTarball/boost/libs/locale/src/boost/locale/win32/collate \
	UnpackedTarball/boost/libs/locale/src/boost/locale/win32/converter \
	UnpackedTarball/boost/libs/locale/src/boost/locale/win32/lcid \
	UnpackedTarball/boost/libs/locale/src/boost/locale/win32/numeric \
	UnpackedTarball/boost/libs/locale/src/boost/locale/win32/win_backend \
))

endif

# vim: set noet sw=4 ts=4:
