# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mDNSResponder))

$(eval $(call gb_UnpackedTarball_set_tarball,mDNSResponder,$(MDNSRESPONDER_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,mDNSResponder,\
	external/mdnsresponder/mDNSResponder_Win32_SOCKET.patch.1 \
	external/mdnsresponder/mDNSResponder_Win32_buildfix.patch.1 \
))

# vim: set noet sw=4 ts=4:
