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

$(eval $(call gb_CppunitTest_CppunitTest,ucb_webdav_neon_opts))

$(eval $(call gb_CppunitTest_use_api,ucb_webdav_neon_opts, \
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,ucb_webdav_neon_opts, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	ucbhelper \
))

$(eval $(call gb_CppunitTest_use_library_objects,ucb_webdav_neon_opts, \
	ucpdav1 \
))

$(eval $(call gb_CppunitTest_use_externals,ucb_webdav_neon_opts,\
	boost_headers \
	libxml2 \
	neon \
	openssl \
))

$(eval $(call gb_CppunitTest_use_custom_headers,ucb_webdav_neon_opts,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_add_exception_objects,ucb_webdav_neon_opts, \
    ucb/qa/cppunit/webdav/webdav_options \
))

$(eval $(call gb_CppunitTest_set_include,ucb_webdav_neon_opts,\
    $$(INCLUDE) \
    -I$(SRCDIR)/ucb/source/ucp/webdav-neon \
))

$(eval $(call gb_CppunitTest_use_static_libraries,ucb_webdav_neon_opts))

# vim: set noet sw=4 ts=4:
