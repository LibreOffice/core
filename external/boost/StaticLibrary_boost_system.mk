# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boost_system))

$(eval $(call gb_StaticLibrary_use_unpacked,boost_system,boost))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,boost_system))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boost_system,\
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_use_external,boost_system,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boost_system,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boost_system,\
	UnpackedTarball/boost/libs/system/src/error_code \
))

# vim: set noet sw=4 ts=4:
