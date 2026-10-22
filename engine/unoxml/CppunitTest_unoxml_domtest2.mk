# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,unoxml_domtest2))

$(eval $(call gb_CppunitTest_add_exception_objects,unoxml_domtest2, \
    unoxml/qa/unit/domtest2 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,unoxml_domtest2))

$(eval $(call gb_CppunitTest_use_components,unoxml_domtest2,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_externals,unoxml_domtest2, \
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,unoxml_domtest2, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sax \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_configuration,unoxml_domtest2))
$(eval $(call gb_CppunitTest_use_ure,unoxml_domtest2))
$(eval $(call gb_CppunitTest_use_vcl,unoxml_domtest2))

# vim: set noet sw=4 ts=4:
