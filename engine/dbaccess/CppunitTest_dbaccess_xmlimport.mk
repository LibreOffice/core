# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,dbaccess_xmlimport))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_xmlimport, \
    dbaccess/qa/unit/xmlimport \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_xmlimport, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,dbaccess_xmlimport))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_xmlimport))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_xmlimport))

$(eval $(call gb_CppunitTest_use_rdb,dbaccess_xmlimport,services))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_xmlimport))

# vim: set noet sw=4 ts=4:
