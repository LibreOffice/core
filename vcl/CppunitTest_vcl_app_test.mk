# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_app_test))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_app_test, \
	vcl/qa/cppunit/app/test_IconThemeInfo \
	vcl/qa/cppunit/app/test_IconThemeScanner \
	vcl/qa/cppunit/app/test_IconThemeSelector \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_app_test, \
	sal \
	vcl \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_app_test, \
	boost_headers \
))

# vim: set noet sw=4 ts=4:
