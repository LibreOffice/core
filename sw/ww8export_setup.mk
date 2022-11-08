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

# template for ww8export tests
define sw_ww8export_test

$(eval $(call gb_CppunitTest_CppunitTest,sw_ww8export$(1)))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_ww8export$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ww8export$(1), \
    sw/qa/extras/ww8export/ww8export$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ww8export$(1), \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    emboleobj \
    sal \
    sfx \
    test \
    unotest \
    utl \
    subsequenttest \
    svl \
    svxcore \
    sw \
	swqahelper \
    tl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ww8export$(1),\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ww8export$(1),\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ww8export$(1),\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ww8export$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_ww8export$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sw_ww8export$(1),services))

ifeq ($(OS),WNT)
# gpgme-w32spawn.exe is needed in workdir/LinkTarget/Executable
$(eval $(call gb_CppunitTest_use_packages,sw_ww8export$(1),\
    $(call gb_Helper_optional,GPGMEPP,gpgmepp)\
))
endif

$(eval $(call gb_CppunitTest_use_configuration,sw_ww8export$(1)))

$(eval $(call gb_CppunitTest_use_packages,sw_ww8export$(1),\
	oox_customshapes \
))

endef

# vim: set noet sw=4 ts=4:
