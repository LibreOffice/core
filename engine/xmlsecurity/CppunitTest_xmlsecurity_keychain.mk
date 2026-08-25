# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,xmlsecurity_keychain))

$(eval $(call gb_CppunitTest_add_exception_objects,xmlsecurity_keychain, \
	xmlsecurity/qa/unit/keychain/keychain \
))

$(eval $(call gb_CppunitTest_use_libraries,xmlsecurity_keychain, \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	sax \
	sfx \
	svl \
	svx \
	subsequenttest \
	test \
	tl \
	ucbhelper \
	unotest \
	utl \
	vcl \
	xmlsecurity \
	xsec_xmlsec \
))

$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_keychain,\
    boost_headers \
    libxml2 \
))

ifneq (,$(ENABLE_NSS))
$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_keychain,\
    nssutil3 \
    nss3 \
))
endif

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,xmlsecurity_keychain,\
	CoreFoundation \
	Security \
))

$(eval $(call gb_CppunitTest_set_include,xmlsecurity_keychain,\
	-I$(SRCDIR)/xmlsecurity/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmlsecurity_keychain))

$(eval $(call gb_CppunitTest_use_ure,xmlsecurity_keychain))
$(eval $(call gb_CppunitTest_use_vcl,xmlsecurity_keychain))

$(eval $(call gb_CppunitTest_use_uiconfigs,xmlsecurity_keychain, \
    svt \
))

$(eval $(call gb_CppunitTest_use_rdb,xmlsecurity_keychain,services))

$(eval $(call gb_CppunitTest_use_configuration,xmlsecurity_keychain))

$(eval $(call gb_CppunitTest_use_custom_headers,xmlsecurity_keychain,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_add_arguments,xmlsecurity_keychain, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
