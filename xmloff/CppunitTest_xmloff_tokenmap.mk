# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,xmloff_tokenmap))

$(eval $(call gb_CppunitTest_add_exception_objects,xmloff_tokenmap, \
	xmloff/qa/unit/tokenmap-test \
))

$(eval $(call gb_CppunitTest_use_custom_headers,xmloff_tokenmap, \
	xmloff/generated \
))

$(eval $(call gb_CppunitTest_use_library_objects,xmloff_tokenmap,xo))

$(eval $(call gb_CppunitTest_use_api,xmloff_tokenmap, \
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,xmloff_tokenmap, \
	basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
    sax \
    svl \
    tl \
    utl \
    vcl \
    $(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
