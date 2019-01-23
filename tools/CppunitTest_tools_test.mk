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

$(eval $(call gb_CppunitTest_CppunitTest,tools_test))

$(eval $(call gb_CppunitTest_use_external,tools_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,tools_test, \
    tools/qa/cppunit/test_bigint \
    tools/qa/cppunit/test_date \
    tools/qa/cppunit/test_time \
    tools/qa/cppunit/test_fract \
    tools/qa/cppunit/test_inetmime \
    tools/qa/cppunit/test_pathutils \
    tools/qa/cppunit/test_reversemap \
    tools/qa/cppunit/test_stream \
    tools/qa/cppunit/test_urlobj \
    tools/qa/cppunit/test_color \
    tools/qa/cppunit/test_rectangle \
    tools/qa/cppunit/test_minmax \
    tools/qa/cppunit/test_100mm2twips \
    tools/qa/cppunit/test_fround \
    tools/qa/cppunit/test_xmlwalker \
))

$(eval $(call gb_CppunitTest_use_sdk_api,tools_test))

$(eval $(call gb_CppunitTest_use_libraries,tools_test, \
    sal \
    tl \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_static_libraries,tools_test, \
    ooopathutils \
))

$(eval $(call gb_CppunitTest_set_include,tools_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/tools/inc \
))

$(eval $(call gb_Library_use_externals,tools_test,\
	libxml2 \
))

# vim: set noet sw=4 ts=4:
