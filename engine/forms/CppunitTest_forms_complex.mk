# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,forms_complex))

$(eval $(call gb_CppunitTest_add_exception_objects,forms_complex, \
    forms/qa/CheckOGroupBoxModel \
))

$(eval $(call gb_CppunitTest_use_libraries,forms_complex, \
    comphelper \
    cppu \
    cppuhelper \
    frm \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,forms_complex))

$(eval $(call gb_CppunitTest_use_ure,forms_complex))
$(eval $(call gb_CppunitTest_use_vcl,forms_complex))

$(eval $(call gb_CppunitTest_use_rdb,forms_complex,services))

$(eval $(call gb_CppunitTest_use_configuration,forms_complex))

# vim: set noet sw=4 ts=4:
