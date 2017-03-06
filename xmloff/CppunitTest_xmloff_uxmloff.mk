# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,xmloff_uxmloff))

$(eval $(call gb_CppunitTest_add_exception_objects,xmloff_uxmloff, \
    xmloff/qa/unit/uxmloff \
	xmloff/qa/unit/tokenmap-test \
))

$(eval $(call gb_CppunitTest_set_include,xmloff_uxmloff,\
    -I$(SRCDIR)/xmloff/inc \
    -I$(SRCDIR)/xmloff/source/style \
    -I$(SRCDIR)/xmloff/source/chart \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_custom_headers,xmloff_uxmloff, \
	xmloff/generated \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmloff_uxmloff))

$(eval $(call gb_CppunitTest_use_ure,xmloff_uxmloff))
$(eval $(call gb_CppunitTest_use_vcl,xmloff_uxmloff))

$(eval $(call gb_CppunitTest_use_library_objects,xmloff_uxmloff,xo))

$(eval $(call gb_CppunitTest_use_externals,xmloff_uxmloff,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,xmloff_uxmloff, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
	sax \
    svl \
	test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_components,xmloff_uxmloff,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

#    uui/util/uui \
#    sfx2/util/sfx \
#    framework/util/fwk \

$(eval $(call gb_CppunitTest_use_configuration,xmloff_uxmloff))

# vim: set noet sw=4 ts=4:
