# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xmlsecurity))

$(eval $(call gb_Library_set_componentfile,xmlsecurity,xmlsecurity/util/xmlsecurity,services))

$(eval $(call gb_Library_set_include,xmlsecurity,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
))

$(eval $(call gb_Library_add_defs,xmlsecurity,\
    -DXMLSECURITY_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_externals,xmlsecurity,\
	boost_headers \
))

$(eval $(call gb_Library_set_precompiled_header,xmlsecurity,xmlsecurity/inc/pch/precompiled_xmlsecurity))

$(eval $(call gb_Library_use_sdk_api,xmlsecurity))

$(eval $(call gb_Library_use_libraries,xmlsecurity,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sax \
	svl \
	sfx \
	svt \
	svxcore \
	tl \
	ucbhelper \
	utl \
	vcl \
	xo \
	i18nlangtag \
	xsec_xmlsec \
))

$(eval $(call gb_Library_add_exception_objects,xmlsecurity,\
	xmlsecurity/source/component/certificatecontainer \
	xmlsecurity/source/dialogs/certificatechooser \
	xmlsecurity/source/dialogs/certificateviewer \
	xmlsecurity/source/dialogs/digitalsignaturesdialog \
	xmlsecurity/source/dialogs/macrosecurity \
	xmlsecurity/source/framework/buffernode \
	xmlsecurity/source/framework/elementcollector \
	xmlsecurity/source/framework/elementmark \
	xmlsecurity/source/framework/saxeventkeeperimpl \
	xmlsecurity/source/framework/securityengine \
	xmlsecurity/source/framework/signaturecreatorimpl \
	xmlsecurity/source/framework/signatureengine \
	xmlsecurity/source/framework/signatureverifierimpl \
	xmlsecurity/source/framework/xmlsignaturetemplateimpl \
	xmlsecurity/source/helper/documentsignaturehelper \
	xmlsecurity/source/helper/documentsignaturemanager \
	xmlsecurity/source/helper/ooxmlsecparser \
	xmlsecurity/source/helper/ooxmlsecexporter \
	xmlsecurity/source/helper/pdfsignaturehelper \
    xmlsecurity/source/helper/UriBindingHelper \
	xmlsecurity/source/helper/xsecctl \
	xmlsecurity/source/helper/xsecparser \
	xmlsecurity/source/helper/xsecsign \
))

ifneq (,$(or $(ENABLE_NSS),$(filter WNT,$(OS))))
$(eval $(call gb_Library_add_exception_objects,xmlsecurity,\
    xmlsecurity/source/component/documentdigitalsignatures \
    xmlsecurity/source/helper/xmlsignaturehelper \
    xmlsecurity/source/helper/xsecverify \
))

$(eval $(call gb_Library_set_componentfile,xmlsecurity,xmlsecurity/util/xmlsecurity_dds,services))
endif

$(eval $(call gb_Library_use_externals,xmlsecurity,\
    libxml2 \
))
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_defs,xmlsecurity,\
    -DXMLSEC_CRYPTO_MSCRYPTO \
))
$(eval $(call gb_Library_use_system_win32_libs,xmlsecurity,\
    crypt32 \
    Ole32 \
    Shell32 \
))
else
ifneq (,$(filter DESKTOP,$(BUILD_TYPE))$(filter ANDROID,$(OS)))
ifeq (TRUE,$(ENABLE_NSS))
$(eval $(call gb_Library_add_defs,xmlsecurity,\
    -DXMLSEC_CRYPTO_NSS \
))
$(eval $(call gb_Library_use_externals,xmlsecurity,\
    nss3 \
    plc4 \
))
endif
endif # BUILD_TYPE=DESKTOP
endif

# vim: set noet sw=4 ts=4:
