# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,openssl))

$(eval $(call gb_UnpackedTarball_set_tarball,openssl,$(OPENSSL_TARBALL),,openssl))

$(eval $(call gb_UnpackedTarball_add_patches,openssl,\
	$(if $(filter MACOSX,$(OS)),,external/openssl/openssllnx.patch) \
	external/openssl/opensslosxppc.patch \
	external/openssl/opensslmingw.patch \
	external/openssl/opensslwnt.patch \
	external/openssl/openssl-1.0.1h-win64.patch.1 \
	external/openssl/opensslsol.patch \
	external/openssl/opensslios.patch \
	external/openssl/openssl-3650-masm.patch.1 \
	external/openssl/openssl-fixbuild.patch.1 \
))

# vim: set noet sw=4 ts=4:
