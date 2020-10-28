# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,filter_textfilterdetect))

$(eval $(call gb_CppunitTest_use_api,filter_textfilterdetect,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,filter_textfilterdetect, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	textfd \
	tl \
	unotest \
	utl \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_textfilterdetect, \
	filter/qa/unit/textfilterdetect \
))

$(eval $(call gb_CppunitTest_use_ure,filter_textfilterdetect))

$(eval $(call gb_CppunitTest_use_vcl,filter_textfilterdetect))

$(eval $(call gb_CppunitTest_use_rdb,filter_textfilterdetect,services))

$(eval $(call gb_CppunitTest_use_configuration,filter_textfilterdetect))

# vim: set noet sw=4 ts=4:
