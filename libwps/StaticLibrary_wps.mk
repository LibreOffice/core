# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,wpslib))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,wpslib))

$(eval $(call gb_StaticLibrary_use_unpacked,wpslib,wps))

$(eval $(call gb_StaticLibrary_use_package,wpslib,libwps_inc))

$(eval $(call gb_StaticLibrary_use_externals,wpslib,\
	wpd \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,wpslib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,wpslib,\
	$(addprefix UnpackedTarball/wps/,$(libwps_SOURCEFILES)) \
))

# vim: set noet sw=4 ts=4:
