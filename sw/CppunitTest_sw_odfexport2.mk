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

$(eval $(call gb_CppunitTest_CppunitTest,sw_odfexport2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_odfexport2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_odfexport2, \
    sw/qa/extras/odfexport/odfexport2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_odfexport2, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
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
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_odfexport2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_odfexport2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_odfexport2,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_odfexport2))
$(eval $(call gb_CppunitTest_use_vcl,sw_odfexport2))

$(eval $(call gb_CppunitTest_use_rdb,sw_odfexport2,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_odfexport2,\
    officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_odfexport2))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_odfexport2, \
    modules/swriter \
    svx \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_odfexport2))

$(eval $(call gb_CppunitTest_add_arguments,sw_odfexport2, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
