# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sax_cpputest))

$(eval $(call gb_CppunitTest_use_api,sax_cpputest,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,sax_cpputest, \
    sax \
    sal \
    comphelper \
    cppu \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sax_cpputest, \
    sax/qa/cppunit/test_converter \
))


# vim: set noet sw=4 ts=4:
