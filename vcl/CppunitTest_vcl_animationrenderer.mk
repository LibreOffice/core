# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_animationrenderer))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_animationrenderer, \
	vcl/qa/cppunit/animationrenderer \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_animationrenderer, \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_animationrenderer))

$(eval $(call gb_CppunitTest_use_ure,vcl_animationrenderer))
$(eval $(call gb_CppunitTest_use_vcl,vcl_animationrenderer))

$(eval $(call gb_CppunitTest_use_components,vcl_animationrenderer,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_animationrenderer))

# vim: set noet sw=4 ts=4:
