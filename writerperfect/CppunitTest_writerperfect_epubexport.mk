# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_epubexport))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_epubexport, \
    writerperfect/qa/unit/EPUBExportTest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,writerperfect_epubexport))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_epubexport, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	subsequenttest \
	test \
	tl \
	unotest \
	utl \
	wpftwriter \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,writerperfect_epubexport,\
    $(if $(ENABLE_WASM_STRIP_EPUB),,epubgen) \
    libxml2 \
    revenge \
))

$(eval $(call gb_CppunitTest_use_external,writerperfect_epubexport,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,writerperfect_epubexport))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_epubexport))
$(eval $(call gb_CppunitTest_use_vcl,writerperfect_epubexport))

$(eval $(call gb_CppunitTest_use_rdb,writerperfect_epubexport,services))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_epubexport))

# vim: set noet sw=4 ts=4:
