# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,forms_unit))

$(eval $(call gb_CppunitTest_use_sdk_api,forms_unit))

$(eval $(call gb_CppunitTest_use_externals,forms_unit, \
    libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,forms_unit, \
    forms/qa/unit/forms_test \
))

$(eval $(call gb_CppunitTest_use_libraries,forms_unit, \
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

$(eval $(call gb_CppunitTest_use_ure,forms_unit))
$(eval $(call gb_CppunitTest_use_vcl,forms_unit))
$(eval $(call gb_CppunitTest_use_rdb,forms_unit,services))
$(eval $(call gb_CppunitTest_use_configuration,forms_unit))

# vim: set noet sw=4 ts=4:
