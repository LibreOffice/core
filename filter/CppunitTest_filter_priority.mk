# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,filter_priority))

$(eval $(call gb_CppunitTest_use_sdk_api,filter_priority))
$(eval $(call gb_CppunitTest_use_ure,filter_priority))
$(eval $(call gb_CppunitTest_use_vcl,filter_priority))

$(eval $(call gb_CppunitTest_use_configuration,filter_priority))

$(eval $(call gb_CppunitTest_use_external,filter_priority,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,filter_priority, \
	comphelper \
	unotest \
	cppuhelper \
	cppu \
	sal \
))

$(eval $(call gb_CppunitTest_use_components,filter_priority,\
	configmgr/source/configmgr \
	filter/source/config/cache/filterconfig1 \
	framework/util/fwk \
	i18npool/util/i18npool \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,filter_priority, \
	filter/qa/cppunit/priority-test \
))

# vim: set noet sw=4 ts=4:
