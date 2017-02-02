# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xsec_gpg))

$(eval $(call gb_Library_set_componentfile,xsec_gpg,xmlsecurity/util/xsec_gpg))

$(eval $(call gb_Library_set_include,xsec_gpg,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
	-I$(SRCDIR)/xmlsecurity/source/gpg \
))

$(eval $(call gb_Library_add_defs,xsec_fw,\
	-DXSECFW_DLLIMPLEMENTATION \
	-DXMLSEC_CRYPTO_GPG \
))

$(eval $(call gb_Library_use_custom_headers,xsec_gpg,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,xsec_gpg))

$(eval $(call gb_Library_set_precompiled_header,xsec_gpg,$(SRCDIR)/xmlsecurity/inc/pch/precompiled_xsec_gpg))

$(eval $(call gb_Library_use_externals,xsec_gpg,gpgme))

$(eval $(call gb_Library_use_libraries,xsec_gpg,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svl \
	tl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,xsec_gpgc,\
	xmlsecurity/source/gpg/CipherContext \
	xmlsecurity/source/gpg/DigestContext \
	xmlsecurity/source/gpg/GpgComponentFactory \
	xmlsecurity/source/gpg/SecurityEnvironment \
	xmlsecurity/source/gpg/SEInitializer \
	xmlsecurity/source/gpg/XMLEncryption \
	xmlsecurity/source/gpg/XMLSecurityContext \
	xmlsecurity/source/gpg/XMLSignature \
))

# vim: set noet sw=4 ts=4:
