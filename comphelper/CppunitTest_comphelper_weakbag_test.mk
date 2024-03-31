# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CppunitTest_CppunitTest,comphelper_weakbag))

$(eval $(call gb_CppunitTest_add_exception_objects,comphelper_weakbag,\
    comphelper/qa/weakbag/test_weakbag \
))

$(eval $(call gb_CppunitTest_use_api,comphelper_weakbag,\
    udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,comphelper_weakbag,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
))

# vim: set noet sw=4 ts=4:
