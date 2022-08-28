# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_import))

$(eval $(call gb_CppunitTest_add_defs,writerperfect_import,\
	-DTEST_DIR=\"writerperfect/qa/unit/data/import\" \
))

$(eval $(call gb_CppunitTest_use_external,writerperfect_import,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,writerperfect_import))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_import,\
	comphelper \
	cppu \
	sal \
	test \
	tl \
	unotest \
	utl \
	wpftqahelper \
))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_import))
$(eval $(call gb_CppunitTest_use_vcl,writerperfect_import))

$(eval $(call gb_CppunitTest_use_rdb,writerperfect_import,services))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_import))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_import,\
	writerperfect/qa/unit/ImportTest \
))

# vim: set noet sw=4 ts=4:
