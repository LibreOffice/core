# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svtools_graphic))

$(eval $(call gb_CppunitTest_use_externals,svtools_graphic,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_use_api,svtools_graphic, \
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,svtools_graphic))

$(eval $(call gb_CppunitTest_use_vcl,svtools_graphic))

$(eval $(call gb_CppunitTest_use_libraries,svtools_graphic, \
	comphelper \
	cppu \
	cppuhelper \
	tl \
	sal \
	svt \
	test \
	unotest \
	vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_configuration,svtools_graphic))

$(eval $(call gb_CppunitTest_use_components,svtools_graphic,\
	configmgr/source/configmgr \
	ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svtools_graphic, \
    svtools/qa/unit/GraphicObjectTest \
))

# vim: set noet sw=4 ts=4:
