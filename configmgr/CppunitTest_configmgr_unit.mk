# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,configmgr_unit))

$(eval $(call gb_CppunitTest_add_exception_objects,configmgr_unit, \
    configmgr/qa/unit/test \
))

$(eval $(call gb_CppunitTest_set_componentfile,configmgr_unit,configmgr/source/configmgr))

$(eval $(call gb_CppunitTest_use_library_objects,configmgr_unit,configmgr))

$(eval $(call gb_CppunitTest_use_sdk_api,configmgr_unit,))

$(eval $(call gb_CppunitTest_use_custom_headers,configmgr_unit,\
        officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_libraries,configmgr_unit, \
	    comphelper \
	    cppu \
	    cppuhelper \
	    sal \
	    salhelper \
	    test \
	    unotest \
	    xmlreader \
		i18nlangtag \
		$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_ure,configmgr_unit))

$(eval $(call gb_CppunitTest_use_configuration,configmgr_unit))

$(eval $(call gb_CppunitTest_use_components,configmgr_unit,\
    i18npool/util/i18npool \
    i18npool/source/search/i18nsearch \
    sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_externals,configmgr_unit,\
    boost_headers \
    dconf \
    icu_headers \
))
