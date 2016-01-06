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

# break symlinks after extracting tarball
# note: escape \; because LO patched make 3.82 cuts off the command otherwise
ifeq ($(OS_FOR_BUILD),WNT)
$(eval $(call gb_UnpackedTarball_set_pre_action,openssl,\
	cd include/openssl && \
	for header in `$(FIND) . -type l` \; do \
		cp --remove-destination `readlink $$$$header` $$$$header \; \
	done && cd -))
endif

$(eval $(call gb_UnpackedTarball_add_patches,openssl,\
	external/openssl/openssllnx.patch \
	external/openssl/opensslmingw.patch \
	external/openssl/opensslwnt.patch \
	external/openssl/openssl-1.0.1g-msvc2012-winxp.patch.1 \
	external/openssl/openssl-1.0.1h-win64.patch.1 \
	external/openssl/opensslsol.patch \
	external/openssl/opensslios.patch \
	external/openssl/opensslosxppc.patch \
	external/openssl/openssl-3650-masm.patch.1 \
))

# vim: set noet sw=4 ts=4:
