# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_draw))

$(eval $(call gb_CppunitTest_use_sdk_api,writerperfect_draw))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_draw,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	tl \
	ucbhelper \
	unotest \
	wpftqahelper \
))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_draw))
$(eval $(call gb_CppunitTest_use_vcl,writerperfect_draw))

$(eval $(call gb_CppunitTest_use_rdb,writerperfect_draw,services))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_draw))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_draw,\
	writerperfect/qa/unit/WpftDrawFilterTest \
))

# vim: set noet sw=4 ts=4:
