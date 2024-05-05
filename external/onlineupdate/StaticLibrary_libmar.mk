# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libmar))

$(eval $(call gb_StaticLibrary_use_unpacked,libmar,onlineupdate))

$(eval $(call gb_StaticLibrary_set_include,libmar,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/src/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/other-licenses/nsis/Contrib/CityHash/cityhash \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,libmar,cpp))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libmar,\
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/src/mar_create \
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/src/mar_extract \
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/src/mar_read \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,libmar, \
    UnpackedTarball/onlineupdate/other-licenses/nsis/Contrib/CityHash/cityhash/city \
))

$(eval $(call gb_StaticLibrary_use_static_libraries,\
	libmarverify \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libmar))

ifeq ($(OS),WNT)
$(eval $(call gb_StaticLibrary_add_defs,libmar, \
    -DXP_WIN \
))
endif

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
