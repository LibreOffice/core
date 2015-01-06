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
	for header in `find . -type l` \; do \
		cp --remove-destination `readlink $$$$header` $$$$header \; \
	done && cd -))
endif

$(eval $(call gb_UnpackedTarball_add_patches,openssl,\
	$(if $(filter LINUX FREEBSD ANDROID,$(OS)),external/openssl/openssllnx.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),external/openssl/opensslmingw.patch) \
	$(if $(filter MSC,$(COM)),external/openssl/opensslwnt.patch) \
	$(if $(filter MSC,$(COM)),external/openssl/openssl-1.0.1g-msvc2012-winxp.patch.1) \
	$(if $(filter MSC,$(COM)),external/openssl/openssl-1.0.1h-win64.patch.1) \
	$(if $(filter SOLARIS,$(OS)),external/openssl/opensslsol.patch) \
	$(if $(filter IOS,$(OS)),external/openssl/opensslios.patch) \
	$(if $(filter MACOSXPOWERPC,$(OS)$(CPUNAME)),external/openssl/opensslosxppc.patch) \
    external/openssl/ubsan.patch.0 \
))

# vim: set noet sw=4 ts=4:
