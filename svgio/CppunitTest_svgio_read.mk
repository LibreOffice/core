# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svgio_read))

$(eval $(call gb_CppunitTest_add_exception_objects,svgio_read,\
    svgio/qa/cppunit/SvgRead \
))

$(eval $(call gb_CppunitTest_set_include,svgio_read,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svgio/inc \
))

$(eval $(call gb_CppunitTest_use_externals,svgio_read,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,svgio_read, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	i18nlangtag \
	sal \
	salhelper \
	sax \
	sot \
	svl \
	svt \
	svx \
	svxcore \
	test \
	tl \
	tk \
	ucbhelper \
	unotest \
	utl \
	vcl \
	xo \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svgio_read))
$(eval $(call gb_CppunitTest_use_ure,svgio_read))
$(eval $(call gb_CppunitTest_use_vcl,svgio_read))
$(eval $(call gb_CppunitTest_use_rdb,svgio_read,services))

$(eval $(call gb_CppunitTest_use_custom_headers,svgio_read,\
        officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,svgio_read))

# vim: set noet sw=4 ts=4:
