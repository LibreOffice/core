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

$(eval $(call gb_CppunitTest_CppunitTest,sw_uiwriter3))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_uiwriter3))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uiwriter3, \
	sw/qa/extras/uiwriter/uiwriter3 \
))

# note: this links msword only for the reason to have an order dependency,
# because "make sw.check" will not see the dependency through services.rdb
$(eval $(call gb_CppunitTest_use_libraries,sw_uiwriter3, \
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	i18nlangtag \
	msword \
	sal \
	sfx \
	svl \
	svt \
	svxcore \
	sw \
	swqahelper \
	test \
	unotest \
	vcl \
	tl \
	utl \
	svx \
))

$(eval $(call gb_CppunitTest_use_externals,sw_uiwriter3,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_uiwriter3,\
	-I$(SRCDIR)/sw/inc \
	-I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/source/uibase/inc \
	-I$(SRCDIR)/sw/source/filter/inc \
	-I$(SRCDIR)/sw/source/filter/html \
	-I$(SRCDIR)/sw/qa/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_uiwriter3,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_uiwriter3))
$(eval $(call gb_CppunitTest_use_vcl,sw_uiwriter3))

$(eval $(call gb_CppunitTest_use_rdb,sw_uiwriter3,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_uiwriter3))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_uiwriter3, \
	modules/swriter \
	sfx \
	svt \
	svx \
))

$(eval $(call gb_CppunitTest_use_packages,sw_uiwriter3, \
	oox_customshapes \
	sfx2_classification \
))

$(call gb_CppunitTest_get_target,sw_uiwriter3): \
	$(call gb_Library_get_target,textconv_dict)

$(eval $(call gb_CppunitTest_use_more_fonts,sw_uiwriter3))

# vim: set noet sw=4 ts=4:
