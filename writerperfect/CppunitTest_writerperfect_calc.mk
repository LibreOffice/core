# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_calc))

$(eval $(call gb_CppunitTest_use_externals,writerperfect_calc,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_use_api,writerperfect_calc,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_calc,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	tl \
	ucbhelper \
	unotest \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_static_libraries,writerperfect_calc,\
	writerperfect_importtestbase \
))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_calc))
$(eval $(call gb_CppunitTest_use_vcl,writerperfect_calc))

$(eval $(call gb_CppunitTest_use_rdb,writerperfect_calc,services))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_calc))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_calc,\
	writerperfect/qa/unit/WpftCalcFilterTest \
))

$(call gb_CppunitTest_get_target,writerperfect_calc): \
    $(call gb_Library_get_target,ipb)

# vim: set noet sw=4 ts=4:
