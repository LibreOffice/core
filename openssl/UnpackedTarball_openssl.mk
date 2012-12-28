# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,openssl))

$(eval $(call gb_UnpackedTarball_set_tarball,openssl,$(OPENSSL_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_patches,openssl,\
	openssl/openssl-asm-fix.patch \
	$(if $(filter LINUX FREEBSD ANDROID,$(OS)),openssl/openssllnx.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),openssl/opensslmingw.patch) \
	$(if $(filter MSC,$(COM)),openssl/opensslwnt.patch) \
	$(if $(filter SOLARIS,$(OS)),openssl/opensslsol.patch) \
	$(if $(filter IOS,$(OS)),openssl/opensslios.patch) \
))

# vim: set noet sw=4 ts=4:
