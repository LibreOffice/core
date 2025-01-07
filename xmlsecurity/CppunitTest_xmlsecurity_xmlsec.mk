# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,xmlsecurity_xmlsec))

$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_xmlsec,\
    boost_headers \
))

ifneq ($(OS),WNT)
ifneq (,$(ENABLE_NSS))
$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_xmlsec,\
    nssutil3 \
    nss3 \
))
endif
endif

$(eval $(call gb_CppunitTest_add_exception_objects,xmlsecurity_xmlsec, \
    xmlsecurity/qa/xmlsec/xmlsec \
))

$(eval $(call gb_CppunitTest_use_libraries,xmlsecurity_xmlsec, \
    comphelper \
    cppu \
    cppuhelper \
    embobj \
    sal \
    sfx \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
    xmlsecurity \
    xsec_xmlsec \
))

$(eval $(call gb_CppunitTest_set_include,xmlsecurity_xmlsec,\
    -I$(SRCDIR)/xmlsecurity/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmlsecurity_xmlsec))

$(eval $(call gb_CppunitTest_use_ure,xmlsecurity_xmlsec))
$(eval $(call gb_CppunitTest_use_vcl,xmlsecurity_xmlsec))

$(eval $(call gb_CppunitTest_use_rdb,xmlsecurity_xmlsec,services))

$(eval $(call gb_CppunitTest_use_custom_headers,xmlsecurity_xmlsec,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,xmlsecurity_xmlsec))

# vim: set noet sw=4 ts=4:
