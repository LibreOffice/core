# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,starmath_qa_cppunit))

$(eval $(call gb_CppunitTest_set_include,starmath_qa_cppunit,\
    $$(INCLUDE) \
    -I$(SRCDIR)/starmath/inc \
))

$(eval $(call gb_CppunitTest_use_external,starmath_qa_cppunit,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,starmath_qa_cppunit))

$(eval $(call gb_CppunitTest_use_library_objects,starmath_qa_cppunit,\
    sm \
))

$(eval $(call gb_CppunitTest_use_libraries,starmath_qa_cppunit,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    sax \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    test \
    tk \
    tl \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,starmath_qa_cppunit,\
    -I$(SRCDIR)/starmath/source \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,starmath_qa_cppunit,\
    starmath/qa/cppunit/test_cursor \
    starmath/qa/cppunit/test_node \
    starmath/qa/cppunit/test_nodetotextvisitors \
    starmath/qa/cppunit/test_parse \
    starmath/qa/cppunit/test_starmath \
))

$(eval $(call gb_CppunitTest_use_ure,starmath_qa_cppunit))
$(eval $(call gb_CppunitTest_use_vcl,starmath_qa_cppunit))

$(eval $(call gb_CppunitTest_use_components,starmath_qa_cppunit,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/source/xstor/xstor \
    package/util/package2 \
    toolkit/util/tk \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    vcl/vcl.common \
))

$(eval $(call gb_CppunitTest_use_configuration,starmath_qa_cppunit))

# vim: set noet sw=4 ts=4:
