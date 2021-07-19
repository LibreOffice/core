# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_sharedresources))

$(eval $(call gb_CppunitTest_set_include,connectivity_sharedresources,\
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_external,connectivity_sharedresources,boost_headers))

$(eval $(call gb_CppunitTest_use_ure,connectivity_sharedresources))
$(eval $(call gb_CppunitTest_use_vcl,connectivity_sharedresources))

$(eval $(call gb_CppunitTest_use_sdk_api,connectivity_sharedresources))

$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_sharedresources, \
	connectivity/qa/connectivity/resource/sharedresources_test \
))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_sharedresources, \
	dbtools \
	sal \
	test \
	utl \
	unotest \
))

$(eval $(call gb_CppunitTest_use_components,connectivity_sharedresources,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_sharedresources))

# vim: set noet sw=4 ts=4:
