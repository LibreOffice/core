# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_commontools))

$(eval $(call gb_CppunitTest_set_include,connectivity_commontools,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_external,connectivity_commontools,boost_headers))

$(eval $(call gb_CppunitTest_use_ure,connectivity_commontools))
$(eval $(call gb_CppunitTest_use_vcl,connectivity_commontools))

$(eval $(call gb_CppunitTest_use_sdk_api,connectivity_commontools))

$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_commontools, \
	connectivity/qa/connectivity/commontools/FValue_test \
))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_commontools, \
	cppu \
	dbtools \
	sal \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_use_components,connectivity_commontools,\
	configmgr/source/configmgr \
))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_commontools))

# vim: set noet sw=4 ts=4:
