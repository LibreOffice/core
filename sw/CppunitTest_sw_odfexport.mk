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

$(eval $(call gb_CppunitTest_CppunitTest,sw_odfexport))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_odfexport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_odfexport, \
    sw/qa/extras/odfexport/odfexport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_odfexport, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
	sfx \
	svl \
	sw \
	swqahelper \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_odfexport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_odfexport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_odfexport,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_odfexport))
$(eval $(call gb_CppunitTest_use_vcl,sw_odfexport))

$(eval $(call gb_CppunitTest_use_rdb,sw_odfexport,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_odfexport,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_odfexport))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_odfexport, \
    modules/swriter \
    svx \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_odfexport))

# vim: set noet sw=4 ts=4:
