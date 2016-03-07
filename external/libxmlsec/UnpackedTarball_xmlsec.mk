# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

xmlsec_patches :=
xmlsec_patches += xmlsec1-configure.patch
xmlsec_patches += xmlsec1-configure-libxml-libxslt.patch
xmlsec_patches += xmlsec1-oldlibtool.patch
xmlsec_patches += xmlsec1-nssdisablecallbacks.patch
xmlsec_patches += xmlsec1-nssmangleciphers.patch
xmlsec_patches += xmlsec1-noverify.patch
xmlsec_patches += xmlsec1-mingw-keymgr-mscrypto.patch
xmlsec_patches += xmlsec1-vc.patch
xmlsec_patches += xmlsec1-1.2.14_fix_extern_c.patch
xmlsec_patches += xmlsec1-android.patch
# Partial backport of <https://github.com/lsh123/xmlsec/commit/6a4968bc33f83aaf61efc0a80333350ce9c372f5>.
xmlsec_patches += xmlsec1-1.2.14-ansi.patch
xmlsec_patches += xmlsec1-customkeymanage.patch.1
xmlsec_patches += xmlsec1-update-config.guess.patch.1
# Upstreamed as <https://github.com/lsh123/xmlsec/commit/7069e2b0ab49679008abedd6d223fb95538b0684>.
xmlsec_patches += xmlsec1-ooxml.patch.1
# Partial backport of <https://github.com/lsh123/xmlsec/commit/a17e8da3a8f56348d71d325aa8d3e6366f13b512>.
xmlsec_patches += xmlsec1-nss-sha256.patch.1
xmlsec_patches += xmlsec1-mscrypto-sha256.patch.1

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xmlsec))

$(eval $(call gb_UnpackedTarball_set_tarball,xmlsec,$(LIBXMLSEC_TARBALL),,libxmlsec))

$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	$(foreach patch,$(xmlsec_patches),external/libxmlsec/$(patch)) \
))

ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	external/libxmlsec/xmlsec1-mingw32.patch \
))
endif

# vim: set noet sw=4 ts=4:
