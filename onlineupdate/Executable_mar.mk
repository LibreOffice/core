# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,mar))

$(eval $(call gb_Executable_set_include,mar,\
	-I$(SRCDIR)/onlineupdate/source/libmar/inc/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/src/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/verify/ \
	-I$(SRCDIR)/onlineupdate/source/libmar/sign/ \
	$$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_libs,mar,\
    ws2_32.lib \
    Crypt32.lib \
))
endif

ifeq ($(filter WNT MACOSX,$(OS)),)
$(eval $(call gb_Executable_use_externals,mar,nss3))
$(eval $(call gb_Executable_add_defs,mar,-DMAR_NSS))
endif

$(eval $(call gb_Executable_add_cobjects,mar,\
	onlineupdate/source/libmar/src/mar_create \
	onlineupdate/source/libmar/src/mar_extract \
	onlineupdate/source/libmar/src/mar_read \
	onlineupdate/source/libmar/sign/nss_secutil \
	onlineupdate/source/libmar/sign/mar_sign \
	onlineupdate/source/libmar/verify/cryptox \
	onlineupdate/source/libmar/verify/mar_verify \
	onlineupdate/source/libmar/tool/mar \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab: */
