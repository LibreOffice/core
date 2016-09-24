# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,io_textinputstream))

$(eval $(call gb_CppunitTest_add_exception_objects,io_textinputstream, \
    io/qa/textinputstream \
))

$(eval $(call gb_CppunitTest_use_libraries,io_textinputstream, \
    cppu \
    cppuhelper \
    sal \
    unotest \
))

$(eval $(call gb_CppunitTest_use_udk_api,io_textinputstream))

$(eval $(call gb_CppunitTest_use_ure,io_textinputstream))

# vim: set noet sw=4 ts=4:
