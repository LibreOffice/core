# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,xmlsec))

$(eval $(call gb_UnpackedTarball_set_tarball,xmlsec,$(LIBXMLSEC_TARBALL),,libxmlsec))

$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	external/libxmlsec/xmlsec1-configure.patch \
	external/libxmlsec/xmlsec1-configure-libxml-libxslt.patch \
	external/libxmlsec/xmlsec1-olderlibxml2.patch \
	external/libxmlsec/xmlsec1-oldlibtool.patch \
	external/libxmlsec/xmlsec1-nssdisablecallbacks.patch \
	external/libxmlsec/xmlsec1-nssmangleciphers.patch \
	external/libxmlsec/xmlsec1-noverify.patch \
	external/libxmlsec/xmlsec1-mingw-keymgr-mscrypto.patch \
	external/libxmlsec/xmlsec1-vc.patch \
	external/libxmlsec/xmlsec1-1.2.14_fix_extern_c.patch \
	external/libxmlsec/xmlsec1-android.patch \
	external/libxmlsec/xmlsec1-1.2.14-ansi.patch \
	external/libxmlsec/xmlsec1-customkeymanage.patch \
	external/libxmlsec/xmlsec1-update-config.guess.patch.1 \
))

$(eval $(call gb_UnpackedTarball_add_file,xmlsec,include/xmlsec/mscrypto/akmngr.h,external/libxmlsec/include/akmngr_mscrypto.h))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,src/mscrypto/akmngr.c,external/libxmlsec/src/akmngr_mscrypto.c))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,include/xmlsec/nss/akmngr.h,external/libxmlsec/include/akmngr_nss.h))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,include/xmlsec/nss/ciphers.h,external/libxmlsec/include/ciphers.h))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,include/xmlsec/nss/tokens.h,external/libxmlsec/include/tokens.h))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,src/nss/akmngr.c,external/libxmlsec/src/akmngr_nss.c))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,src/nss/keywrapers.c,external/libxmlsec/src/keywrapers.c))
$(eval $(call gb_UnpackedTarball_add_file,xmlsec,src/nss/tokens.c,external/libxmlsec/src/tokens.c))

ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_UnpackedTarball_add_patches,xmlsec,\
	external/libxmlsec/xmlsec1-mingw32.patch \
))
endif

# vim: set noet sw=4 ts=4:
