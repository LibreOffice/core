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
	external/openssl/CVE-2013-6449.patch \
	external/openssl/CVE-2013-6450.patch \
	external/openssl/CVE-2013-4353.patch \
	external/openssl/CVE-2014-0160.patch \
	external/openssl/CVE-2010-5298.patch \
	external/openssl/CVE-2014-0195.patch \
	external/openssl/CVE-2014-0198.patch \
	external/openssl/CVE-2014-0221.patch \
	external/openssl/CVE-2014-0224.patch \
	external/openssl/CVE-2014-3470.patch \
	external/openssl/CVE-2014-3505.patch \
	external/openssl/CVE-2014-3506.patch \
	external/openssl/CVE-2014-3507.patch \
	external/openssl/CVE-2014-3508.patch \
	external/openssl/CVE-2014-3509.patch \
	external/openssl/CVE-2014-3510.patch \
	external/openssl/CVE-2014-3511.patch \
	external/openssl/CVE-2014-3513.patch \
	external/openssl/CVE-2014-3567.patch \
	external/openssl/CVE-2014-3566.patch \
	$(if $(filter LINUX FREEBSD ANDROID,$(OS)),external/openssl/openssllnx.patch) \
	$(if $(filter WNTGCC,$(OS)$(COM)),external/openssl/opensslmingw.patch) \
	$(if $(filter MSC,$(COM)),external/openssl/opensslwnt.patch) \
	$(if $(filter SOLARIS,$(OS)),external/openssl/opensslsol.patch) \
	$(if $(filter IOS,$(OS)),external/openssl/opensslios.patch) \
))

# vim: set noet sw=4 ts=4:
