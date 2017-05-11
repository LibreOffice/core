# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xsec_gpg))

ifneq ($(OS),WNT)
$(eval $(call gb_Library_set_componentfile,xsec_gpg,xmlsecurity/util/xsec_gpg))
endif

$(eval $(call gb_Library_set_include,xsec_gpg,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
	-I$(SRCDIR)/xmlsecurity/source/gpg \
	-I$(call gb_UnpackedTarball_get_dir,xmlsec/include \
)))

$(eval $(call gb_Library_add_defs,xsec_gpg,\
	-DXMLSEC_NO_XSLT \
	-DXMLSEC_CRYPTO_NSS \
	-DXSECGPG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_custom_headers,xsec_gpg,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,xsec_gpg))

$(eval $(call gb_Library_set_precompiled_header,xsec_gpg,$(SRCDIR)/xmlsecurity/inc/pch/precompiled_xsec_gpg))

$(eval $(call gb_Library_use_packages,xsec_gpg,\
	xmlsec \
))
$(eval $(call gb_Library_use_externals,xsec_gpg,\
	boost_headers \
	libxml2 \
	nss3 \
	gpgmepp))

$(eval $(call gb_Library_use_libraries,xsec_gpg,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svl \
	tl \
	xo \
))

$(eval $(call gb_Library_add_defs,xsec_gpg,\
	-DXMLSEC_CRYPTO_NSS \
))
$(eval $(call gb_Library_add_libs,xsec_gpg,\
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/nss/.libs/libxmlsec1-nss.a \
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/.libs/libxmlsec1.a \
))

$(eval $(call gb_Library_add_exception_objects,xsec_gpg,\
	xmlsecurity/source/gpg/CertificateImpl \
	xmlsecurity/source/gpg/CipherContext \
	xmlsecurity/source/gpg/DigestContext \
	xmlsecurity/source/gpg/GpgComponentFactory \
	xmlsecurity/source/gpg/SecurityEnvironment \
	xmlsecurity/source/gpg/SEInitializer \
	xmlsecurity/source/gpg/XMLEncryption \
	xmlsecurity/source/gpg/XMLSecurityContext \
	xmlsecurity/source/gpg/XMLSignature \
	xmlsecurity/source/gpg/xmlsignature_gpgimpl \
	xmlsecurity/source/xmlsec/biginteger \
	xmlsecurity/source/xmlsec/certificateextension_certextn \
	xmlsecurity/source/xmlsec/errorcallback \
	xmlsecurity/source/xmlsec/saxhelper \
	xmlsecurity/source/xmlsec/serialnumberadapter \
	xmlsecurity/source/xmlsec/xmldocumentwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlelementwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlstreamio \
))

# vim: set noet sw=4 ts=4:
