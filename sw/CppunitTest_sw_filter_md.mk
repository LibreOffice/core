# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_filter_md))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_filter_md))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_filter_md, \
    sw/qa/filter/md/md \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_filter_md, \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    sw \
    swqahelper \
    test \
    tl \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_filter_md,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_filter_md,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_filter_md,\
        offapi \
        udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_filter_md))
$(eval $(call gb_CppunitTest_use_vcl,sw_filter_md))

$(eval $(call gb_CppunitTest_use_rdb,sw_filter_md,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_filter_md))

# vim: set noet sw=4 ts=4:
