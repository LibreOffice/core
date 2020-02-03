# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,lotuswordpro_import_test))

$(eval $(call gb_CppunitTest_use_external,lotuswordpro_import_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,lotuswordpro_import_test, \
    lotuswordpro/qa/cppunit/import_test \
))

$(eval $(call gb_CppunitTest_use_libraries,lotuswordpro_import_test, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,lotuswordpro_import_test))

$(eval $(call gb_CppunitTest_use_ure,lotuswordpro_import_test))
$(eval $(call gb_CppunitTest_use_vcl,lotuswordpro_import_test))

$(eval $(call gb_CppunitTest_use_rdb,lotuswordpro_import_test,services))

$(eval $(call gb_CppunitTest_use_configuration,lotuswordpro_import_test))

# vim: set noet sw=4 ts=4:
