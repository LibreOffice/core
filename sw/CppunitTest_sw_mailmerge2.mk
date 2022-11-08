# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_mailmerge2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_mailmerge2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_mailmerge2, \
    sw/qa/extras/mailmerge/mailmerge2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_mailmerge2, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    subsequenttest \
    svl \
    sw \
	swqahelper \
    test \
    tl \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_mailmerge2, \
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_api,sw_mailmerge2,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_rdb,sw_mailmerge2,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_mailmerge2))
$(eval $(call gb_CppunitTest_use_ure,sw_mailmerge2))
$(eval $(call gb_CppunitTest_use_vcl,sw_mailmerge2))

$(eval $(call gb_CppunitTest_set_include,sw_mailmerge2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_mailmerge2,\
	modules/swriter \
))

# vim: set noet sw=4 ts=4:
