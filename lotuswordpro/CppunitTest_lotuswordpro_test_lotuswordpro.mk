# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,lotuswordpro_test_lotuswordpro))

$(eval $(call gb_CppunitTest_use_external,lotuswordpro_test_lotuswordpro,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,lotuswordpro_test_lotuswordpro, \
    lotuswordpro/qa/cppunit/test_lotuswordpro \
))

$(eval $(call gb_CppunitTest_use_libraries,lotuswordpro_test_lotuswordpro, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,lotuswordpro_test_lotuswordpro,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,lotuswordpro_test_lotuswordpro))

$(eval $(call gb_CppunitTest_use_components,lotuswordpro_test_lotuswordpro,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    lotuswordpro/util/lwpfilter \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,lotuswordpro_test_lotuswordpro))

# vim: set noet sw=4 ts=4:
