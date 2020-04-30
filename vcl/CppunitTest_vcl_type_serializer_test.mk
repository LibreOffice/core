# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_type_serializer_test))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_type_serializer_test, \
    vcl/qa/cppunit/TypeSerializerTest \
))

$(eval $(call gb_CppunitTest_use_external,vcl_type_serializer_test,boost_headers))

$(eval $(call gb_CppunitTest_set_include,vcl_type_serializer_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_type_serializer_test, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svt \
    test \
    tl \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_type_serializer_test))
$(eval $(call gb_CppunitTest_use_rdb,vcl_type_serializer_test,services))
$(eval $(call gb_CppunitTest_use_ure,vcl_type_serializer_test))
$(eval $(call gb_CppunitTest_use_vcl,vcl_type_serializer_test))
$(eval $(call gb_CppunitTest_use_configuration,vcl_type_serializer_test))

# vim: set noet sw=4 ts=4:
