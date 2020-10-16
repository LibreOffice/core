# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_wpftimport))

$(eval $(call gb_CppunitTest_set_include,writerperfect_wpftimport,\
	-I$(SRCDIR)/writerperfect/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_externals,writerperfect_wpftimport,\
	odfgen \
	revenge \
))

$(eval $(call gb_CppunitTest_use_sdk_api,writerperfect_wpftimport))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_wpftimport,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	unotest \
	utl \
	vcl \
	wpftqahelper \
	writerperfect \
	xo \
))

$(eval $(call gb_CppunitTest_use_external,writerperfect_wpftimport,boost_headers))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_wpftimport))
$(eval $(call gb_CppunitTest_use_vcl,writerperfect_wpftimport))

$(eval $(call gb_CppunitTest_use_rdbs,writerperfect_wpftimport,\
	services \
))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_wpftimport))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_wpftimport,\
	writerperfect/qa/unit/DrawingImportTest \
	writerperfect/qa/unit/PresentationImportTest \
	writerperfect/qa/unit/SpreadsheetImportTest \
	writerperfect/qa/unit/TextImportTest \
	writerperfect/qa/unit/wpftimport \
))

# vim: set noet sw=4 ts=4:
