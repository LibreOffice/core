# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,comphelper_test))

$(eval $(call gb_CppunitTest_add_exception_objects,comphelper_test, \
    comphelper/qa/string/test_string \
    comphelper/qa/container/testifcontainer \
    comphelper/qa/unit/test_hash \
    comphelper/qa/unit/base64_test \
    comphelper/qa/unit/types_test \
    comphelper/qa/unit/test_guards \
    comphelper/qa/unit/test_traceevent \
))

$(eval $(call gb_CppunitTest_use_sdk_api,comphelper_test))

$(eval $(call gb_CppunitTest_use_libraries,comphelper_test, \
    comphelper \
    cppuhelper \
    cppu \
    sal \
))

ifeq ($(TLS),NSS)
$(eval $(call gb_CppunitTest_use_externals,comphelper_test,\
       plc4 \
       nss3 \
))
endif

# vim: set noet sw=4 ts=4:
