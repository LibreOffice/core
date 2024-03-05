# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sax_parser))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sax_parser))

$(eval $(call gb_CppunitTest_add_exception_objects,sax_parser, \
	sax/qa/cppunit/parser \
))

$(eval $(call gb_CppunitTest_use_libraries,sax_parser, \
	comphelper \
	cppu \
	cppuhelper \
	unotest \
	sal \
	sax \
	test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sax_parser))

$(eval $(call gb_CppunitTest_use_ure,sax_parser))
$(eval $(call gb_CppunitTest_use_vcl,sax_parser))

$(eval $(call gb_CppunitTest_use_components,sax_parser,\
	configmgr/source/configmgr \
	framework/util/fwk \
	i18npool/util/i18npool \
	sfx2/util/sfx \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,sax_parser))

# vim: set noet sw=4 ts=4:
