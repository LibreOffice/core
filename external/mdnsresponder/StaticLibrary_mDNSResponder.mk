# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,mDNSResponder))

$(eval $(call gb_StaticLibrary_set_warnings_not_errors,mDNSResponder))

$(eval $(call gb_StaticLibrary_use_unpacked,mDNSResponder,mDNSResponder))

$(eval $(call gb_StaticLibrary_set_include,mDNSResponder,\
	-I$(call gb_UnpackedTarball_get_dir,mDNSResponder)/mDNSShared \
	$$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_add_defs,mDNSResponder,\
	-DWIN32_LEAN_AND_MEAN \
	-D_WINSOCK_DEPRECATED_NO_WARNINGS \
	-DUSE_TCP_LOOPBACK \
	-DNOT_HAVE_SA_LEN \
))

$(eval $(call gb_StaticLibrary_add_generated_cobjects,mDNSResponder,\
	UnpackedTarball/mDNSResponder/mDNSShared/DebugServices \
	UnpackedTarball/mDNSResponder/mDNSShared/GenLinkedList \
	UnpackedTarball/mDNSResponder/mDNSShared/dnssd_clientlib \
	UnpackedTarball/mDNSResponder/mDNSShared/dnssd_clientstub \
	UnpackedTarball/mDNSResponder/mDNSShared/dnssd_ipc \
	UnpackedTarball/mDNSResponder/mDNSWindows/DLL/dllmain \
))

# vim: set noet sw=4 ts=4:
