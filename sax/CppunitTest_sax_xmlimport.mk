# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sax_xmlimport))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sax_xmlimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sax_xmlimport, \
sax/qa/cppunit/xmlimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sax_xmlimport, \
    test \
    cppu \
    comphelper \
    sal \
    cppuhelper \
    sax \
    unotest \
    utl \
    salhelper \
))

$(eval $(call gb_CppunitTest_use_api,sax_xmlimport,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sax_xmlimport))
$(eval $(call gb_CppunitTest_use_vcl,sax_xmlimport))

$(eval $(call gb_CppunitTest_set_include,sax_xmlimport,\
    -I$(SRCDIR)/sax/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_components,sax_xmlimport,\
    configmgr/source/configmgr \
    sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    uui/util/uui \
))

$(eval $(call gb_CppunitTest_use_configuration,sax_xmlimport))

# vim: set noet sw=4 ts=4:
