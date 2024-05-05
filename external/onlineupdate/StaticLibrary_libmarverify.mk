# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,libmarverify))

$(eval $(call gb_StaticLibrary_use_unpacked,libmarverify,onlineupdate))

$(eval $(call gb_StaticLibrary_set_include,libmarverify,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/src/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	$$(INCLUDE) \
))

ifeq ($(OS),WNT)

$(eval $(call gb_StaticLibrary_add_defs,libmarverify, \
    -DXP_WIN \
))

else

$(eval $(call gb_StaticLibrary_add_defs,libmarverify,\
	-DMAR_NSS \
))

$(eval $(call gb_StaticLibrary_use_externals,libmarverify,\
	nss3 \
))

endif

$(eval $(call gb_StaticLibrary_add_generated_cobjects,libmarverify,\
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/verify/mar_verify \
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/verify/cryptox \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,libmarverify))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
