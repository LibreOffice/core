# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_a11y))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_a11y, \
	sc/qa/extras/accessibility/basics \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_a11y, \
	sal \
	cppu \
	subsequenttest \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_api,sc_a11y,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_a11y))
$(eval $(call gb_CppunitTest_use_rdb,sc_a11y,services))
$(eval $(call gb_CppunitTest_use_ure,sc_a11y))
$(eval $(call gb_CppunitTest_use_vcl,sc_a11y))

$(eval $(call gb_CppunitTest_use_instdir_configuration,sc_a11y))
$(eval $(call gb_CppunitTest_use_common_configuration,sc_a11y))

# vim: set noet sw=4 ts=4:
