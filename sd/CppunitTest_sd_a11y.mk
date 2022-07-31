# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sd_a11y))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_a11y, \
	sd/qa/unit/a11y/layout \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_a11y, \
	sal \
	cppu \
	subsequenttest \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_api,sd_a11y,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_a11y))
$(eval $(call gb_CppunitTest_use_rdb,sd_a11y,services))
$(eval $(call gb_CppunitTest_use_ure,sd_a11y))
$(eval $(call gb_CppunitTest_use_vcl,sd_a11y))

$(eval $(call gb_CppunitTest_use_instdir_configuration,sd_a11y))
$(eval $(call gb_CppunitTest_use_common_configuration,sd_a11y))

# vim: set noet sw=4 ts=4:
