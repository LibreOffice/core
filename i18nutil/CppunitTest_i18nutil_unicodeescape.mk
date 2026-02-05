# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,i18nutil_unicodeescape))

$(eval $(call gb_CppunitTest_use_external,i18nutil_unicodeescape,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,i18nutil_unicodeescape, \
    i18nutil/qa/cppunit/test_unicodeescape \
))

$(eval $(call gb_CppunitTest_use_libraries,i18nutil_unicodeescape, \
    cppu \
    cppuhelper \
    i18nutil \
    sal \
))

# vim: set noet sw=4 ts=4:
