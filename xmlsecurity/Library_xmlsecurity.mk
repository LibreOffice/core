# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xmlsecurity))

$(eval $(call gb_Library_set_componentfile,xmlsecurity,xmlsecurity/util/xmlsecurity))

$(eval $(call gb_Library_set_include,xmlsecurity,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
))

$(eval $(call gb_Library_add_defs,xmlsecurity,\
    -DXMLSECURITY_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_externals,xmlsecurity,\
	boost_headers \
	gpgme \
))

$(eval $(call gb_Library_set_precompiled_header,xmlsecurity,$(SRCDIR)/xmlsecurity/inc/pch/precompiled_xmlsecurity))

$(eval $(call gb_Library_use_sdk_api,xmlsecurity))

$(eval $(call gb_Library_use_libraries,xmlsecurity,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sax \
	svl \
	svt \
	svxcore \
	tl \
	ucbhelper \
	utl \
	vcl \
	xo \
	i18nlangtag \
	xsec_fw \
	xsec_xmlsec \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,xmlsecurity,\
	xmlsecurity/source/component/certificatecontainer \
	xmlsecurity/source/component/documentdigitalsignatures \
	xmlsecurity/source/component/registerservices \
	xmlsecurity/source/dialogs/certificatechooser \
	xmlsecurity/source/dialogs/certificateviewer \
	xmlsecurity/source/dialogs/digitalsignaturesdialog \
	xmlsecurity/source/dialogs/macrosecurity \
	xmlsecurity/source/dialogs/resourcemanager \
	xmlsecurity/source/gpg/CertificateImpl \
	xmlsecurity/source/gpg/CipherContext \
	xmlsecurity/source/gpg/DigestContext \
	xmlsecurity/source/gpg/GpgComponentFactory \
	xmlsecurity/source/gpg/SecurityEnvironment \
	xmlsecurity/source/gpg/SEInitializer \
	xmlsecurity/source/gpg/XMLEncryption \
	xmlsecurity/source/gpg/XMLSecurityContext \
	xmlsecurity/source/gpg/XMLSignature \
	xmlsecurity/source/helper/documentsignaturehelper \
	xmlsecurity/source/helper/documentsignaturemanager \
	xmlsecurity/source/helper/ooxmlsecparser \
	xmlsecurity/source/helper/ooxmlsecexporter \
	xmlsecurity/source/helper/pdfsignaturehelper \
	xmlsecurity/source/helper/xmlsignaturehelper2 \
	xmlsecurity/source/helper/xmlsignaturehelper \
	xmlsecurity/source/helper/xsecctl \
	xmlsecurity/source/helper/xsecparser \
	xmlsecurity/source/helper/xsecsign \
	xmlsecurity/source/helper/xsecverify \
	xmlsecurity/source/pdfio/pdfdocument \
))

$(eval $(call gb_Library_use_externals,xmlsecurity,\
    libxml2 \
))
ifeq ($(OS)-$(COM),WNT-MSC)
$(eval $(call gb_Library_add_defs,xmlsecurity,\
    -DXMLSEC_CRYPTO_MSCRYPTO \
))
$(eval $(call gb_Library_use_system_win32_libs,xmlsecurity,\
    crypt32 \
))
else
ifneq (,$(filter DESKTOP,$(BUILD_TYPE)))
$(eval $(call gb_Library_add_defs,xmlsecurity,\
    -DXMLSEC_CRYPTO_NSS \
))
$(eval $(call gb_Library_use_externals,xmlsecurity,\
    nss3 \
    plc4 \
))
endif # BUILD_TYPE=DESKTOP
endif

# vim: set noet sw=4 ts=4:
