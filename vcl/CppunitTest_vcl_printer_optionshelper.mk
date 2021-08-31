# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_printer_optionshelper))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_printer_optionshelper, \
    vcl/qa/cppunit/printer/optionshelper \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_printer_optionshelper,\
	boost_headers \
	glm_headers \
))
ifeq ($(DISABLE_GUI),)
$(eval $(call gb_CppunitTest_use_externals,vcl_printer_optionshelper,\
     epoxy \
 ))
endif

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_printer_optionshelper))

$(eval $(call gb_CppunitTest_use_libraries,vcl_printer_optionshelper, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svt \
	test \
	unotest \
	utl \
	tl \
	vcl \
))

$(eval $(call gb_CppunitTest_set_include,vcl_bitmap_test,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_printer_optionshelper))

$(eval $(call gb_CppunitTest_use_ure,vcl_printer_optionshelper))
$(eval $(call gb_CppunitTest_use_vcl,vcl_printer_optionshelper))

$(eval $(call gb_CppunitTest_use_rdb,vcl_printer_optionshelper,services))

$(eval $(call gb_CppunitTest_use_configuration,vcl_printer_optionshelper))

# vim: set noet sw=4 ts=4:
