# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,toolkit_a11y))

$(eval $(call gb_CppunitTest_add_exception_objects,toolkit_a11y, \
	toolkit/qa/cppunit/a11y/AccessibleStatusBarTest \
	toolkit/qa/cppunit/a11y/AccessibilityTools \
	toolkit/qa/cppunit/a11y/XAccessibleComponentTester \
	toolkit/qa/cppunit/a11y/XAccessibleContextTester \
	toolkit/qa/cppunit/a11y/XAccessibleEventBroadcasterTester \
	toolkit/qa/cppunit/a11y/XAccessibleExtendedComponentTester \
))

$(eval $(call gb_CppunitTest_use_libraries,toolkit_a11y, \
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

$(eval $(call gb_CppunitTest_use_api,toolkit_a11y,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_externals,toolkit_a11y,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_sdk_api,toolkit_a11y))

$(eval $(call gb_CppunitTest_use_ure,toolkit_a11y))

$(eval $(call gb_CppunitTest_use_configuration,toolkit_a11y))

$(eval $(call gb_CppunitTest_use_vcl,toolkit_a11y))

$(eval $(call gb_CppunitTest_use_rdb,toolkit_a11y,services))

#~ $(eval $(call gb_CppunitTest_use_components,toolkit_a11y,\
#~ 	comphelper/util/comphelp \
#~ 	configmgr/source/configmgr \
#~ 	framework/util/fwk \
#~ 	i18npool/util/i18npool \
#~ 	sfx2/util/sfx \
#~ 	toolkit/util/tk \
#~ 	ucb/source/core/ucb1 \
#~ 	ucb/source/ucp/file/ucpfile1 \
#~ ))

# vim: set noet sw=4 ts=4:
