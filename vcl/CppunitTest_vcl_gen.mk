# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,vcl_gen))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_gen, \
	vcl/qa/cppunit/gen/gen \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_gen, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sfx \
	subsequenttest \
	test \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_external,vcl_gen,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_gen))

$(eval $(call gb_CppunitTest_use_ure,vcl_gen))
$(eval $(call gb_CppunitTest_use_vcl_non_headless,vcl_gen))

$(eval $(call gb_CppunitTest_use_rdb,vcl_gen,services))

$(eval $(call gb_CppunitTest_use_configuration,vcl_gen))

# vim: set noet sw=4 ts=4:
