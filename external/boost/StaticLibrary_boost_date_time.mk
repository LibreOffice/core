# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boost_date_time))

$(eval $(call gb_StaticLibrary_use_unpacked,boost_date_time,boost))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,boost_date_time))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boost_date_time,\
	-DBOOST_ALL_NO_LIB \
))

# Needed when building against MSVC in C++17 mode, as
# workdir/UnpackedTarball/boost/boost/numeric/conversion/detail/converter.hpp uses
# std::unary_function:
$(eval $(call gb_StaticLibrary_add_defs,boost_date_time, \
    -D_HAS_AUTO_PTR_ETC=1 \
))

$(eval $(call gb_StaticLibrary_use_external,boost_date_time,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boost_date_time,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_date_time,\
	UnpackedTarball/boost/libs/date_time/src/gregorian/gregorian_types \
	UnpackedTarball/boost/libs/date_time/src/gregorian/greg_month \
))

# vim: set noet sw=4 ts=4:
