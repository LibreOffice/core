# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

xmlsec_patches :=
xmlsec_patches += xmlsec1-configure.patch.1
xmlsec_patches += xmlsec1-oldlibtool.patch.1
xmlsec_patches += xmlsec1-nssdisablecallbacks.patch.1
xmlsec_patches += xmlsec1-nssmangleciphers.patch.1
xmlsec_patches += xmlsec1-noverify.patch.1
xmlsec_patches += xmlsec1-mingw-keymgr-mscrypto.patch.1
xmlsec_patches += xmlsec1-vc.patch.1
# Upstreamed as <https://github.com/lsh123/xmlsec/commit/727f1d3f1fa9c23d6d356b233c6fa274b5dd7d71>.
xmlsec_patches += xmlsec1-vs2015.patch.1
xmlsec_patches += xmlsec1-1.2.14_fix_extern_c.patch.1
xmlsec_patches += xmlsec1-customkeymanage.patch.1
# Upstreamed as <https://github.com/lsh123/xmlsec/commit/7069e2b0ab49679008abedd6d223fb95538b0684>.
xmlsec_patches += xmlsec1-ooxml.patch.1

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
