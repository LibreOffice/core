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

define sw_ooxmlexport_libraries
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	sal \
	sfx \
	sw \
	test \
	tl \
	unotest \
	utl \
	vcl \
	svxcore
endef

# template for ooxmlexport tests (there are several so that they can be run in parallel)
define sw_ooxmlexport_test

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmlexport$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmlexport$(1), \
    sw/qa/extras/ooxmlexport/ooxmlexport$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmlexport$(1), \
	$(sw_ooxmlexport_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmlexport$(1),\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmlexport$(1),\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ooxmlexport$(1),\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmlexport$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmlexport$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sw_ooxmlexport$(1),services))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmlexport$(1)))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_ooxmlexport$(1),\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_packages,sw_ooxmlexport$(1),\
	oox_customshapes \
	oox_generated \
))

$(call gb_CppunitTest_get_target,sw_ooxmlexport$(1)) : $(call gb_Library_get_target,iti)

$(eval $(call gb_CppunitTest_use_more_fonts,sw_ooxmlexport$(1)))

endef

# vim: set noet sw=4 ts=4:
