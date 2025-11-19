# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_idle_task))

$(eval $(call gb_CppunitTest_set_include,vcl_idle_task,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_idle_task, \
    vcl/qa/cppunit/IdleTask \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_idle_task, \
    comphelper \
    sal \
    salhelper \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_idle_task))

$(eval $(call gb_CppunitTest_use_ure,vcl_idle_task))
$(eval $(call gb_CppunitTest_use_vcl,vcl_idle_task))

$(eval $(call gb_CppunitTest_use_rdb,vcl_idle_task,services))

$(eval $(call gb_CppunitTest_use_configuration,vcl_idle_task))

# vim: set noet sw=4 ts=4:
