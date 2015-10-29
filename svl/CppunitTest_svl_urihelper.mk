# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svl_urihelper))

$(eval $(call gb_CppunitTest_use_external,svl_urihelper,boost_headers))

$(eval $(call gb_CppunitTest_use_api,svl_urihelper, \
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_components,svl_urihelper, \
    ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svl_urihelper, \
svl/qa/unit/test_URIHelper \
))

$(eval $(call gb_CppunitTest_use_libraries,svl_urihelper, \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    svl \
    tl \
    utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_ure,svl_urihelper))

$(eval $(call gb_CppunitTest_use_components,svl_urihelper,\
    i18npool/util/i18npool \
))

# vim: set noet sw=4 ts=4:
