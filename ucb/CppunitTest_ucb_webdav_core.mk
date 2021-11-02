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

$(eval $(call gb_CppunitTest_CppunitTest,ucb_webdav_core))

$(eval $(call gb_CppunitTest_use_sdk_api,ucb_webdav_core))

$(eval $(call gb_CppunitTest_use_libraries,ucb_webdav_core, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	ucbhelper \
	tl \
))

$(eval $(call gb_CppunitTest_use_library_objects,ucb_webdav_core, \
	ucpdav1 \
))

$(eval $(call gb_CppunitTest_use_externals,ucb_webdav_core,\
	boost_headers \
	libxml2 \
	curl \
))

$(eval $(call gb_CppunitTest_use_custom_headers,ucb_webdav_core,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_add_exception_objects,ucb_webdav_core, \
    ucb/qa/cppunit/webdav/webdav_resource_access \
    ucb/qa/cppunit/webdav/webdav_propfindcache \
    ucb/qa/cppunit/webdav/webdav_options \
    ucb/qa/cppunit/webdav/webdav_local_neon \
))

$(eval $(call gb_CppunitTest_set_include,ucb_webdav_core,\
    $$(INCLUDE) \
    -I$(SRCDIR)/ucb/source/ucp/webdav-curl \
))

# vim: set noet sw=4 ts=4:
