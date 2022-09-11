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

$(eval $(call gb_CppunitTest_CppunitTest,sw_odffeatures))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_odffeatures))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_odffeatures, \
    sw/qa/extras/odfimport/odffeatures \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_odffeatures, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sw \
    swqahelper \
    test \
    unotest \
    vcl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_odffeatures,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_odffeatures,\
    -I$(SRCDIR)/sw/inc \
	-I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_odffeatures,\
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_odffeatures))
$(eval $(call gb_CppunitTest_use_vcl,sw_odffeatures))

$(eval $(call gb_CppunitTest_use_rdb,sw_odffeatures,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_odffeatures))

$(eval $(call gb_CppunitTest_add_arguments,sw_odffeatures, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
