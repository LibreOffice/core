# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# This file contains the unit test definition for class in the sources/core/tox subfolder
# The macro which defines the main method is contained in test_ToxWhitespaceStripper.cxx

$(eval $(call gb_CppunitTest_CppunitTest,sw_tox_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_tox_test, \
	sw/qa/cppunit/tox/test_ToxWhitespaceStripper \
	sw/qa/cppunit/tox/test_ToxLinkProcessor \
	sw/qa/cppunit/tox/test_ToxTextGenerator \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_tox_test, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svt \
	sw \
	test \
	unotest \
	vcl \
	tl \
	utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_tox_test, \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_api,sw_tox_test,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_set_include,sw_tox_test,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    $$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
