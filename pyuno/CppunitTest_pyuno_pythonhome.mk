# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,pyuno_pythonhome))

$(eval $(call gb_CppunitTest_add_exception_objects,pyuno_pythonhome, \
    pyuno/qa/cppunit/testpythonhome \
))

$(eval $(call gb_CppunitTest_use_libraries,pyuno_pythonhome, \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    tl \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,pyuno_pythonhome,\
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_api,pyuno_pythonhome,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_components,pyuno_pythonhome,\
    pyuno/source/loader/pythonloader \
))

$(eval $(call gb_CppunitTest_use_ure,pyuno_pythonhome))
$(eval $(call gb_CppunitTest_use_vcl,pyuno_pythonhome))
$(eval $(call gb_CppunitTest_use_rdb,pyuno_pythonhome,services))
$(eval $(call gb_CppunitTest_use_configuration,pyuno_pythonhome))

# vim: set noet sw=4 ts=4:
