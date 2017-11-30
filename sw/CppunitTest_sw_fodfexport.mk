# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_fodfexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_fodfexport, \
	sw/qa/extras/fodfexport/fodfexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_fodfexport, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sfx \
	sw \
	test \
	tl \
	unotest \
	utl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_fodfexport,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_fodfexport,\
	-I$(SRCDIR)/sw/inc \
	-I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_fodfexport))

$(eval $(call gb_CppunitTest_use_ure,sw_fodfexport))
$(eval $(call gb_CppunitTest_use_vcl,sw_fodfexport))

$(eval $(call gb_CppunitTest_use_rdb,sw_fodfexport,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_fodfexport,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_fodfexport))

# vim: set noet sw=4 ts=4:
