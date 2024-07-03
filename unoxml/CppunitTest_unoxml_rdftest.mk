# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,unoxml_rdftest))

$(eval $(call gb_CppunitTest_add_exception_objects,unoxml_rdftest, \
    unoxml/qa/unit/rdftest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,unoxml_rdftest))

$(eval $(call gb_CppunitTest_use_rdb,unoxml_rdftest,services))

$(eval $(call gb_CppunitTest_use_externals,unoxml_rdftest, \
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,unoxml_rdftest, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sax \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_configuration,unoxml_rdftest))
$(eval $(call gb_CppunitTest_use_ure,unoxml_rdftest))
$(eval $(call gb_CppunitTest_use_vcl,unoxml_rdftest))

# vim: set noet sw=4 ts=4:
