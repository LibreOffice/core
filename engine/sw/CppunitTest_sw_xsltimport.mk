# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_xsltimport))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_xsltimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_xsltimport, \
    sw/qa/extras/xsltimport/xsltimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_xsltimport, \
    comphelper \
    cppu \
	cppuhelper \
	i18nlangtag \
    sal \
    sfx \
    subsequenttest \
    sw \
	swqahelper \
    test \
	tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_xsltimport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_xsltimport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
	-I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_xsltimport,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_xsltimport))
$(eval $(call gb_CppunitTest_use_vcl,sw_xsltimport))

$(eval $(call gb_CppunitTest_use_rdb,sw_xsltimport,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_xsltimport))

$(eval $(call gb_CppunitTest_use_packages,sw_xsltimport,\
	filter_xslt \
))

# vim: set noet sw=4 ts=4: