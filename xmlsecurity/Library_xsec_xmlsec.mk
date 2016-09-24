# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xsec_xmlsec))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_set_componentfile,xsec_xmlsec,xmlsecurity/util/xsec_xmlsec.windows))
else
$(eval $(call gb_Library_set_componentfile,xsec_xmlsec,xmlsecurity/util/xsec_xmlsec))
endif

$(eval $(call gb_Library_set_include,xsec_xmlsec,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
	-I$(SRCDIR)/xmlsecurity/source/xmlsec \
	-I$(call gb_UnpackedTarball_get_dir,xmlsec/include) \
))

$(eval $(call gb_Library_use_custom_headers,xsec_xmlsec,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,xsec_xmlsec))

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_NO_XSLT \
))

$(eval $(call gb_Library_set_precompiled_header,xsec_xmlsec,$(SRCDIR)/xmlsecurity/inc/pch/precompiled_xsec_xmlsec))

$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svl \
	tl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_packages,xsec_xmlsec,\
	xmlsec \
))
$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	boost_headers \
	libxml2 \
	nss3 \
))

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/biginteger \
	xmlsecurity/source/xmlsec/certificateextension_certextn \
	xmlsecurity/source/xmlsec/errorcallback \
	xmlsecurity/source/xmlsec/saxhelper \
	xmlsecurity/source/xmlsec/serialnumberadapter \
	xmlsecurity/source/xmlsec/xmldocumentwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlelementwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlstreamio \
	xmlsecurity/source/xmlsec/xsec_xmlsec \
	xmlsecurity/source/xmlsec/nss/ciphercontext \
	xmlsecurity/source/xmlsec/nss/digestcontext \
	xmlsecurity/source/xmlsec/nss/nssinitializer \
	xmlsecurity/source/xmlsec/nss/xsec_nss \
))

ifeq ($(OS)-$(COM),WNT-MSC)

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_CRYPTO_MSCRYPTO \
))


$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	$(call gb_UnpackedTarball_get_dir,xmlsec)/win32/binaries/libxmlsec-mscrypto.lib \
	$(call gb_UnpackedTarball_get_dir,xmlsec)/win32/binaries/libxmlsec.lib \
))

$(eval $(call gb_Library_use_system_win32_libs,xsec_xmlsec,\
	crypt32 \
	advapi32 \
))

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/mscrypt/sanextension_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/securityenvironment_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/seinitializer_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/x509certificate_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/xmlencryption_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/xmlsecuritycontext_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/xmlsignature_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/xsec_mscrypt \
))

else

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_CRYPTO_NSS \
))

ifeq ($(OS)-$(COM),WNT-GCC)
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/nss/.libs/libxmlsec1-nss.dll.a \
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/.libs/libxmlsec1.dll.a \
))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/openssl/.libs/libxmlsec1-openssl.a \
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/.libs/libxmlsec1.a \
))
else
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/nss/.libs/libxmlsec1-nss.a \
	$(call gb_UnpackedTarball_get_dir,xmlsec)/src/.libs/libxmlsec1.a \
))
endif

$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	plc4 \
))

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/nss/sanextension_nssimpl \
	xmlsecurity/source/xmlsec/nss/secerror \
	xmlsecurity/source/xmlsec/nss/securityenvironment_nssimpl \
	xmlsecurity/source/xmlsec/nss/seinitializer_nssimpl \
	xmlsecurity/source/xmlsec/nss/x509certificate_nssimpl \
	xmlsecurity/source/xmlsec/nss/xmlencryption_nssimpl \
	xmlsecurity/source/xmlsec/nss/xmlsecuritycontext_nssimpl \
	xmlsecurity/source/xmlsec/nss/xmlsignature_nssimpl \
))

endif # ifeq ($(OS)-$(COM),WNT-GCC)

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
