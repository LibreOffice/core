# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,ucb_gdrive))

$(eval $(call gb_CppunitTest_use_api,ucb_gdrive,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,ucb_gdrive))
$(eval $(call gb_CppunitTest_use_vcl,ucb_gdrive))

$(eval $(call gb_CppunitTest_use_rdb,ucb_gdrive,services))

$(eval $(call gb_CppunitTest_use_custom_headers,ucb_gdrive,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,ucb_gdrive))

$(eval $(call gb_CppunitTest_use_libraries,ucb_gdrive,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	ucbhelper \
	ucpgdrive \
	utl \
))

$(eval $(call gb_CppunitTest_use_externals,ucb_gdrive,\
	boost_headers \
	curl \
))

$(eval $(call gb_CppunitTest_add_exception_objects,ucb_gdrive,\
	ucb/qa/cppunit/test_gdrive_json \
	ucb/qa/cppunit/test_gdrive_api_client \
	ucb/qa/cppunit/test_gdrive_provider \
	ucb/qa/cppunit/test_gdrive_mock_server \
	ucb/qa/cppunit/test_gdrive_integration \
	ucb/qa/cppunit/test_gdrive_complete \
	ucb/qa/cppunit/test_gdrive_content \
	ucb/qa/cppunit/test_gdrive_performance \
))

# vim: set noet sw=4 ts=4: