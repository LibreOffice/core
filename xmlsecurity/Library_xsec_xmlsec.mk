# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xsec_xmlsec))

$(eval $(call gb_Library_set_componentfiles,xsec_xmlsec, \
    $(if $(ENABLE_GPGMEPP),xmlsecurity/util/xsec_xmlsec_gpg) \
    $(if $(or $(ENABLE_NSS),$(filter WNT,$(OS))),xmlsecurity/util/xsec_xmlsec_nss_mscrypt) \
    $(if $(ENABLE_NSS),xmlsecurity/util/xsec_xmlsec_nss) \
    xmlsecurity/util/xsec_xmlsec \
,services))

$(eval $(call gb_Library_set_include,xsec_xmlsec,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
	-I$(SRCDIR)/xmlsecurity/source/gpg \
	-I$(SRCDIR)/xmlsecurity/source/xmlsec \
	-I$(call gb_UnpackedTarball_get_dir,xmlsec/include) \
))

$(eval $(call gb_Library_use_custom_headers,xsec_xmlsec,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,xsec_xmlsec))

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_NO_XSLT \
	-DXSECXMLSEC_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,xsec_xmlsec,xmlsecurity/inc/pch/precompiled_xsec_xmlsec))

$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svl \
	tl \
	xo \
	utl \
))

ifeq ($(SYSTEM_XMLSEC),)
$(eval $(call gb_Library_use_packages,xsec_xmlsec,\
	xmlsec \
))
endif

$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	boost_headers \
	$(if $(ENABLE_GPGMEPP),gpgmepp) \
	libxml2 \
	xmlsec \
))

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/biginteger \
	xmlsecurity/source/xmlsec/certificateextension_certextn \
	xmlsecurity/source/xmlsec/errorcallback \
	xmlsecurity/source/xmlsec/saxhelper \
	xmlsecurity/source/xmlsec/xmldocumentwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlelementwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlsec_init \
	xmlsecurity/source/xmlsec/xmlstreamio \
))

ifeq ($(ENABLE_GPGMEPP),TRUE)
$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/gpg/CertificateImpl \
	xmlsecurity/source/gpg/CipherContext \
	xmlsecurity/source/gpg/DigestContext \
	xmlsecurity/source/gpg/SecurityEnvironment \
	xmlsecurity/source/gpg/SEInitializer \
	xmlsecurity/source/gpg/XMLEncryption \
	xmlsecurity/source/gpg/XMLSecurityContext \
	xmlsecurity/source/gpg/xmlsignature_gpgimpl \
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_CRYPTO_MSCRYPTO \
))

$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	$(call gb_UnpackedTarball_get_dir,xmlsec)/win32/binaries/libxmlsec-mscng.lib \
	$(call gb_UnpackedTarball_get_dir,xmlsec)/win32/binaries/libxmlsec.lib \
))

$(eval $(call gb_Library_use_system_win32_libs,xsec_xmlsec,\
	crypt32 \
	advapi32 \
	ncrypt \
))

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/mscrypt/akmngr \
	xmlsecurity/source/xmlsec/mscrypt/sanextension_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/securityenvironment_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/seinitializer_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/x509certificate_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/xmlsecuritycontext_mscryptimpl \
	xmlsecurity/source/xmlsec/mscrypt/xmlsignature_mscryptimpl \
))

ifeq ($(ENABLE_NSS),TRUE)

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/nss/ciphercontext \
	xmlsecurity/source/xmlsec/nss/digestcontext \
	xmlsecurity/source/xmlsec/nss/nssinitializer \
))

# nss3 after static libs to appease --as-needed linkers
$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	nss3 \
))

endif

else # !$(OS),WNT

ifeq ($(SYSTEM_XMLSEC),)
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
       $(call gb_UnpackedTarball_get_dir,xmlsec)/src/.libs/libxmlsec1.a \
))
endif

ifeq ($(ENABLE_NSS),TRUE)

ifeq ($(SYSTEM_XMLSEC),)
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
       $(call gb_UnpackedTarball_get_dir,xmlsec)/src/nss/.libs/libxmlsec1-nss.a \
))
endif

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/nss/ciphercontext \
	xmlsecurity/source/xmlsec/nss/digestcontext \
	xmlsecurity/source/xmlsec/nss/nssinitializer \
	xmlsecurity/source/xmlsec/nss/sanextension_nssimpl \
	xmlsecurity/source/xmlsec/nss/secerror \
	xmlsecurity/source/xmlsec/nss/securityenvironment_nssimpl \
	xmlsecurity/source/xmlsec/nss/seinitializer_nssimpl \
	xmlsecurity/source/xmlsec/nss/x509certificate_nssimpl \
	xmlsecurity/source/xmlsec/nss/xmlsecuritycontext_nssimpl \
	xmlsecurity/source/xmlsec/nss/xmlsignature_nssimpl \
))

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_CRYPTO_NSS \
))

$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	plc4 \
))
# nss3 after static libs to appease --as-needed linkers
$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	nss3 \
))

else # ! $(ENABLE_NSS)

ifeq ($(ENABLE_OPENSSL),TRUE)
$(eval $(call gb_Library_use_external,xsec_xmlsec,openssl))
endif

endif # !$(ENABLE_NSS)

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	-ldl \
))
endif

endif # !$(OS),WNT

# vim: set noet sw=4 ts=4:
