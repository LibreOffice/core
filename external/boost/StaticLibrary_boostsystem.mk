# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,boostsystem))

$(eval $(call gb_StaticLibrary_use_unpacked,boostsystem,boost))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,boostsystem))

# disable "auto link" "feature" on MSVC
$(eval $(call gb_StaticLibrary_add_defs,boostsystem,\
	-DBOOST_ALL_NO_LIB \
))

$(eval $(call gb_StaticLibrary_use_external,boostsystem,boost_headers))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,boostsystem,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,boostsystem,\
	UnpackedTarball/boost/libs/system/src/error_code \
))

# vim: set noet sw=4 ts=4:
