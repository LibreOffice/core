# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svl_passwordcontainer))

$(eval $(call gb_CppunitTest_add_exception_objects,svl_passwordcontainer, \
    svl/qa/unit/PasswordContainer \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svl_passwordcontainer))

$(eval $(call gb_CppunitTest_use_components,svl_passwordcontainer,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
	svl/source/passwordcontainer/passwordcontainer \
	ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_libraries,svl_passwordcontainer, \
    comphelper \
    cppu \
    cppuhelper \
    passwordcontainer \
    sal \
    svl \
    sax \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_configuration,svl_passwordcontainer))
$(eval $(call gb_CppunitTest_use_ure,svl_passwordcontainer))
$(eval $(call gb_CppunitTest_use_vcl,svl_passwordcontainer))

# vim: set noet sw=4 ts=4:
