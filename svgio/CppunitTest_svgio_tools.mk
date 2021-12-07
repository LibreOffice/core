# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svgio_tools))

$(eval $(call gb_CppunitTest_add_exception_objects,svgio_tools,\
    svgio/qa/cppunit/SvgNumberTest \
))

$(eval $(call gb_CppunitTest_set_include,svgio_tools,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svgio/inc \
))

$(eval $(call gb_CppunitTest_use_externals,svgio_tools,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_library_objects,svgio_tools,\
    svgio \
))

$(eval $(call gb_CppunitTest_use_libraries,svgio_tools,\
    basegfx \
    drawinglayer \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    sax \
    svt \
    tk \
    tl \
    vcl \
))

# vim: set noet sw=4 ts=4:
