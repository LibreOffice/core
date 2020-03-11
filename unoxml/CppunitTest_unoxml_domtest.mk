# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,unoxml_domtest))

$(eval $(call gb_CppunitTest_add_exception_objects,unoxml_domtest, \
    unoxml/qa/unit/domtest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,unoxml_domtest))

$(eval $(call gb_CppunitTest_use_components,unoxml_domtest,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_externals,unoxml_domtest, \
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,unoxml_domtest, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sax \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_configuration,unoxml_domtest))
$(eval $(call gb_CppunitTest_use_ure,unoxml_domtest))
$(eval $(call gb_CppunitTest_use_vcl,unoxml_domtest))

# vim: set noet sw=4 ts=4:
