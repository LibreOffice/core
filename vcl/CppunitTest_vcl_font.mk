# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_font))

$(eval $(call gb_CppunitTest_set_include,vcl_font,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_font, \
	vcl/qa/cppunit/font \
	vcl/qa/cppunit/physicalfontface \
	vcl/qa/cppunit/physicalfontfacecollection \
	vcl/qa/cppunit/physicalfontfamily \
	vcl/qa/cppunit/physicalfontcollection \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_font,\
	boost_headers \
	harfbuzz \
	graphite \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_font, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	svt \
	test \
	tl \
	tk \
	unotest \
	utl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_font))

$(eval $(call gb_CppunitTest_use_ure,vcl_font))
$(eval $(call gb_CppunitTest_use_vcl,vcl_font))

$(eval $(call gb_CppunitTest_use_components,vcl_font,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
	ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_font))

# vim: set noet sw=4 ts=4:
