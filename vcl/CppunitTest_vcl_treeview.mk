# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_treeview))

$(eval $(call gb_CppunitTest_set_include,vcl_treeview,\
	$$(INCLUDE) \
	-I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_treeview, \
	vcl/qa/cppunit/treeviewtest \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_treeview, \
	boost_headers \
	harfbuzz \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_treeview, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svt \
	test \
	tk \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_treeview))

$(eval $(call gb_CppunitTest_use_ure,vcl_treeview))
$(eval $(call gb_CppunitTest_use_vcl,vcl_treeview))

$(eval $(call gb_CppunitTest_use_components,vcl_treeview,\
	configmgr/source/configmgr \
	framework/util/fwk \
	i18npool/util/i18npool \
	sax/source/expatwrap/expwrap \
	sfx2/util/sfx \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	unotools/util/utl \
	unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_treeview))

$(eval $(call gb_CppunitTest_use_uiconfigs,vcl_treeview, \
    vcl \
))

# vim: set noet sw=4 ts=4:
