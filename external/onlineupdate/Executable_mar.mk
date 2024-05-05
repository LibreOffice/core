# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,mar))

$(eval $(call gb_Executable_use_unpacked,mar,onlineupdate))

$(eval $(call gb_Executable_set_include,mar,\
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/src/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/verify/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/onlineupdate/source/libmar/sign/ \
	-I$(gb_UnpackedTarball_workdir)/onlineupdate/include/onlineupdate \
	$$(INCLUDE) \
))

$(eval $(call gb_Executable_use_static_libraries,mar,\
    libmar \
    libmarverify \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Executable_add_defs,mar,\
    -DXP_WIN \
))

$(eval $(call gb_Executable_add_libs,mar,\
    ws2_32.lib \
    Crypt32.lib \
))

endif

$(eval $(call gb_Executable_use_externals,mar,\
	nss3 \
))

$(eval $(call gb_Executable_add_defs,mar,\
	-DMAR_NSS \
))

$(eval $(call gb_Executable_add_defs,mar,\
	-DMOZ_APP_VERSION=\"$(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR).$(LIBO_VERSION_MICRO).$(LIBO_VERSION_PATCH)\" \
	-DMAR_CHANNEL_ID=\"LOOnlineUpdater\" \
))

$(eval $(call gb_Executable_add_generated_cobjects,mar,\
	UnpackedTarball/onlineupdate/nsprpub/lib/libc/src/strdup \
	UnpackedTarball/onlineupdate/nsprpub/lib/libc/src/strlen \
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/sign/nss_secutil \
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/sign/mar_sign \
	UnpackedTarball/onlineupdate/onlineupdate/source/libmar/tool/mar \
))

$(eval $(call gb_Executable_set_warnings_disabled,mar))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
