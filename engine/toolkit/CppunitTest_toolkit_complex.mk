# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,toolkit_complex))

$(eval $(call gb_CppunitTest_add_exception_objects,toolkit_complex, \
	toolkit/qa/cppunit/complex/GridControl \
))

$(eval $(call gb_CppunitTest_use_libraries,toolkit_complex, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	subsequenttest \
	test \
	tk \
	tl \
	utl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_api,toolkit_complex,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_externals,toolkit_complex,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_sdk_api,toolkit_complex))

$(eval $(call gb_CppunitTest_use_ure,toolkit_complex))

$(eval $(call gb_CppunitTest_use_configuration,toolkit_complex))

$(eval $(call gb_CppunitTest_use_vcl,toolkit_complex))

$(eval $(call gb_CppunitTest_use_rdb,toolkit_complex,services))

# vim: set noet sw=4 ts=4:
