# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_impress))

$(eval $(call gb_CppunitTest_use_externals,writerperfect_impress,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_use_api,writerperfect_impress,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_impress, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    ucbhelper \
    unotest \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_impress))

$(eval $(call gb_CppunitTest_use_rdb,writerperfect_impress,services))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_impress))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_impress, \
    writerperfect/qa/unit/wpftimpress \
))

# vim: set noet sw=4 ts=4:
