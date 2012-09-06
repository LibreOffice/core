# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boostdatetime))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,boostdatetime))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boostdatetime,\
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_use_unpacked,boostdatetime,boost))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boostdatetime,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boostdatetime,\
	UnpackedTarball/boost/libs/date_time/src/gregorian/date_generators \
	UnpackedTarball/boost/libs/date_time/src/gregorian/gregorian_types \
	UnpackedTarball/boost/libs/date_time/src/gregorian/greg_month \
	UnpackedTarball/boost/libs/date_time/src/gregorian/greg_weekday \
))

# vim: set noet sw=4 ts=4:
