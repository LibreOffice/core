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

$(eval $(call gb_CppunitTest_CppunitTest,sw_xhtmlexport))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_xhtmlexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_xhtmlexport, \
    sw/qa/extras/htmlexport/xhtmlexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_xhtmlexport, \
    comphelper \
    cppu \
	cppuhelper \
	i18nlangtag \
    sal \
    sfx \
    sw \
	swqahelper \
    test \
	tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_xhtmlexport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_xhtmlexport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
	-I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_xhtmlexport,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_xhtmlexport))
$(eval $(call gb_CppunitTest_use_vcl,sw_xhtmlexport))

$(eval $(call gb_CppunitTest_use_rdb,sw_xhtmlexport,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_xhtmlexport))

$(eval $(call gb_CppunitTest_use_packages,sw_xhtmlexport, \
    filter_xhtml \
    filter_xslt \
))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_xhtmlexport, \
    svx \
))

# vim: set noet sw=4 ts=4:
