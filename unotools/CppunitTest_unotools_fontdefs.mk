# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,unotools_fontdefs))

$(eval $(call gb_CppunitTest_use_external,unotools_fontdefs,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,unotools_fontdefs))

$(eval $(call gb_CppunitTest_add_exception_objects,unotools_fontdefs, \
    unotools/qa/unit/testGetEnglishSearchName \
))

$(eval $(call gb_CppunitTest_use_libraries,unotools_fontdefs, \
	comphelper \
	cppu \
	cppuhelper \
	tl \
	sal \
	svt \
	utl \
))

# vim: set noet sw=4 ts=4:
