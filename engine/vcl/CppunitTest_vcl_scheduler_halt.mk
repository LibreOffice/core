# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_scheduler_halt))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_scheduler_halt, \
    vcl/qa/cppunit/SchedulerHalt \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_scheduler_halt, \
    comphelper \
    sal \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_scheduler_halt))

$(eval $(call gb_CppunitTest_use_ure,vcl_scheduler_halt))
$(eval $(call gb_CppunitTest_use_vcl,vcl_scheduler_halt))

$(eval $(call gb_CppunitTest_use_rdb,vcl_scheduler_halt,services))

$(eval $(call gb_CppunitTest_use_configuration,vcl_scheduler_halt))

# vim: set noet sw=4 ts=4:
