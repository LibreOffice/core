# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,dbaccess_hsqldb_test))

$(eval $(call gb_CppunitTest_use_external,dbaccess_hsqldb_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_hsqldb_test, \
    dbaccess/qa/unit/hsqldb \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_hsqldb_test, \
    affine_uno_uno \
    comphelper \
    cppu \
    cppuhelper \
    dbaxml \
    dbtools \
    jvmfwk \
    sal \
    subsequenttest \
    utl \
    test \
    tk \
    unotest \
    xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,dbaccess_hsqldb_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_hsqldb_test))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_hsqldb_test))

$(eval $(call gb_CppunitTest_use_java_ure,dbaccess_hsqldb_test))

$(eval $(call gb_CppunitTest_use_rdb,dbaccess_hsqldb_test,services))

$(eval $(call gb_CppunitTest_use_jars,dbaccess_hsqldb_test,\
	sdbc_hsqldb \
))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_hsqldb_test))

$(call gb_CppunitTest_get_target,dbaccess_hsqldb_test): \
    $(call gb_AllLangResTarget_get_target,ofa)

# vim: set noet sw=4 ts=4:
