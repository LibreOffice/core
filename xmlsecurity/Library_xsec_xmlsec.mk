# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
))

$(eval $(call gb_Library_use_sdk_api,xsec_xmlsec))

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_NO_XSLT \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	xmlsec1 \
))
else
$(eval $(call gb_Library_use_static_libraries,xsec_xmlsec,\
	xmlsec1 \
))
endif


$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svl \
	tl \
	xo \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_externals,xsec_xmlsec,\
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,xsec_xmlsec,\
	xmlsecurity/source/xmlsec/biginteger \
	xmlsecurity/source/xmlsec/certificateextension_xmlsecimpl \
	xmlsecurity/source/xmlsec/certvalidity \
	xmlsecurity/source/xmlsec/diagnose \
	xmlsecurity/source/xmlsec/errorcallback \
	xmlsecurity/source/xmlsec/saxhelper \
	xmlsecurity/source/xmlsec/xmldocumentwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlelementwrapper_xmlsecimpl \
	xmlsecurity/source/xmlsec/xmlstreamio \
	xmlsecurity/source/xmlsec/xsec_xmlsec \
	xmlsecurity/source/xmlsec/nss/ciphercontext \
	xmlsecurity/source/xmlsec/nss/digestcontext \
	xmlsecurity/source/xmlsec/nss/nssinitializer \
	xmlsecurity/source/xmlsec/nss/xsec_nss \
))

ifeq ($(GUI)$(CROSS_COMPILING),WNT)

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DXMLSEC_CRYPTO_MSCRYPTO \
))

$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	xmlsec1-mscrypto \
))

$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
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

ifeq ($(GUI)$(CROSS_COMPILING),WNTYES)
$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	xmlsec1-nss \
))
else
$(eval $(call gb_Library_use_static_libraries,xsec_xmlsec,\
	xmlsec1-nss \
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

endif # ifeq ($(GUI)$(CROSS_COMPILING),WNT)

ifeq ($(SYSTEM_NSS),YES)

$(eval $(call gb_Library_set_include,xsec_xmlsec,\
	$$(INCLUDE) \
	$(NSS_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,xsec_xmlsec,\
	-DSYSTEM_MOZILLA \
))

$(eval $(call gb_Library_add_libs,xsec_xmlsec,\
	$(NSS_LIBS) \
))

else

$(eval $(call gb_Library_set_include,xsec_xmlsec,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc/mozilla/nspr \
	-I$(OUTDIR)/inc/mozilla/nss \
))

$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	nspr4 \
	nss3 \
))

endif # ifeq ($(SYSTEM_NSS),YES)

ifeq ($(OS),SOLARIS)
$(eval $(call gb_Library_use_libraries,xsec_xmlsec,\
	dl \
))
endif

# vim: set noet sw=4 ts=4:
