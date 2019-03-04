# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,toolkit))

$(eval $(call gb_CppunitTest_add_exception_objects,toolkit, \
	toolkit/qa/cppunit/Dialog \
	toolkit/qa/cppunit/UnitConversion \
))

$(eval $(call gb_CppunitTest_use_libraries,toolkit, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	tk \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_api,toolkit,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_externals,toolkit,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_ure,toolkit))

$(eval $(call gb_CppunitTest_use_configuration,toolkit))

$(eval $(call gb_CppunitTest_use_vcl,toolkit))

$(eval $(call gb_CppunitTest_use_components,toolkit,\
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	framework/util/fwk \
	i18npool/util/i18npool \
	sfx2/util/sfx \
	toolkit/util/tk \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
))

# vim: set noet sw=4 ts=4:
