# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_animationrenderers))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_animationrenderers, \
	vcl/qa/cppunit/animationrenderers \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_animationrenderers, \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_animationrenderers))

$(eval $(call gb_CppunitTest_use_ure,vcl_animationrenderers))
$(eval $(call gb_CppunitTest_use_vcl,vcl_animationrenderers))

$(eval $(call gb_CppunitTest_use_components,vcl_animationrenderers,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_animationrenderers))

# vim: set noet sw=4 ts=4:
