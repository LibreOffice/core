# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,libcmis))

$(eval $(call gb_UnpackedTarball_set_tarball,libcmis,$(LIBCMIS_TARBALL)))

# 0001-cid-1545775-COPY_INSTEAD_OF_MOVE.patch is upstreamed

$(eval $(call gb_UnpackedTarball_add_patches,libcmis,\
	external/libcmis/0001-cid-1545775-COPY_INSTEAD_OF_MOVE.patch \
	external/libcmis/0001-Fix-warning-C4589-when-building-with-MSVC.patch \
	external/libcmis/http-session-cleanup.patch.1 \
	external/libcmis/factory-no-retry-ssl.patch.1 \
	external/libcmis/sharepoint-auth.patch.1 \
	external/libcmis/exceptions.patch.1 \
	external/libcmis/0001-cppcheck-operatorEqVarError-in-src-libcmis-http-sess.patch \
	external/libcmis/0001-Take-into-account-m_CurlInitProtocolsFunction-in-cop.patch \
	external/libcmis/initprotocols.patch.1 \
	external/libcmis/0001-Fix-boost-1.86-breakage.patch \
))

# vim: set noet sw=4 ts=4:
