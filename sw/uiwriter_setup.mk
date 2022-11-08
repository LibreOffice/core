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

define sw_uiwriter_libraries
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    msword \
    sal \
    sfx \
    subsequenttest \
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
    svx
endef

# template for uiwriter tests (there are several so that they can be run in parallel)
define sw_uiwriter_test

$(eval $(call gb_CppunitTest_CppunitTest,sw_uiwriter$(1)))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_uiwriter$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uiwriter$(1), \
    sw/qa/extras/uiwriter/uiwriter$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_uiwriter$(1), \
	$(sw_uiwriter_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_uiwriter$(1),\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_uiwriter$(1),\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_set_include,sw_uiwriter$(1),\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/source/filter/html \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_uiwriter$(1),\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_uiwriter$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_uiwriter$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sw_uiwriter$(1),services))

$(eval $(call gb_CppunitTest_use_configuration,sw_uiwriter$(1)))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_uiwriter$(1),\
    modules/swriter \
    sfx \
    svt \
    svx \
))

$(eval $(call gb_CppunitTest_use_packages,sw_uiwriter$(1),\
    oox_customshapes \
    sfx2_classification \
))

$(call gb_CppunitTest_get_target,sw_uiwriter$(1)) : $(call gb_Library_get_target,textconv_dict)

$(eval $(call gb_CppunitTest_use_more_fonts,sw_uiwriter$(1)))

$(eval $(call gb_CppunitTest_add_arguments,sw_uiwriter$(1), \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

endef

# vim: set noet sw=4 ts=4:
