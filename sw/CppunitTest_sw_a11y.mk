# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_a11y))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_a11y, \
	sw/qa/extras/accessibility/basics \
	sw/qa/extras/accessibility/dialogs \
	sw/qa/extras/accessibility/unicode \
	sw/qa/extras/accessibility/tdf155705 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_a11y, \
	sal \
	comphelper \
	cppu \
	cppuhelper \
	cui \
	subsequenttest \
	test \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_api,sw_a11y,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_a11y))
$(eval $(call gb_CppunitTest_use_rdb,sw_a11y,services))
$(eval $(call gb_CppunitTest_use_ure,sw_a11y))
$(eval $(call gb_CppunitTest_use_vcl,sw_a11y))

$(eval $(call gb_CppunitTest_use_instdir_configuration,sw_a11y))
$(eval $(call gb_CppunitTest_use_common_configuration,sw_a11y))

$(eval $(call gb_CppunitTest_use_packages,sw_a11y, \
    postprocess_images \
))

# vim: set noet sw=4 ts=4:
