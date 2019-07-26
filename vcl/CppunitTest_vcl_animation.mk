# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_animation))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_animation, \
	vcl/qa/cppunit/animation \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_animation, \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_udk_api,vcl_animation))

$(eval $(call gb_CppunitTest_use_ure,vcl_animation))
$(eval $(call gb_CppunitTest_use_vcl,vcl_animation))

$(eval $(call gb_CppunitTest_use_components,vcl_animation,\
	configmgr/source/configmgr \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_animation))

# vim: set noet sw=4 ts=4:
