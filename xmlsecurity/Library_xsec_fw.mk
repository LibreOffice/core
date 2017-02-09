# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,xsec_fw))

$(eval $(call gb_Library_set_include,xsec_fw,\
	$$(INCLUDE) \
	-I$(SRCDIR)/xmlsecurity/inc \
))

$(eval $(call gb_Library_add_defs,xsec_fw,\
    -DXSECFW_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,xsec_fw,$(SRCDIR)/xmlsecurity/inc/pch/precompiled_xsec_fw))

$(eval $(call gb_Library_use_sdk_api,xsec_fw))

$(eval $(call gb_Library_use_libraries,xsec_fw,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,xsec_fw,\
	xmlsecurity/source/framework/buffernode \
	xmlsecurity/source/framework/decryptorimpl \
	xmlsecurity/source/framework/elementcollector \
	xmlsecurity/source/framework/elementmark \
	xmlsecurity/source/framework/encryptionengine \
	xmlsecurity/source/framework/saxeventkeeperimpl \
	xmlsecurity/source/framework/securityengine \
	xmlsecurity/source/framework/signaturecreatorimpl \
	xmlsecurity/source/framework/signatureengine \
	xmlsecurity/source/framework/signatureverifierimpl \
	xmlsecurity/source/framework/xmlencryptiontemplateimpl \
	xmlsecurity/source/framework/xmlsignaturetemplateimpl \
))

# vim: set noet sw=4 ts=4:
