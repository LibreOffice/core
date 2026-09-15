# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_roundtrip_diff))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_roundtrip_diff))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_roundtrip_diff, \
    sw/qa/extras/roundtrip/roundtrip_diff \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_roundtrip_diff, \
	$(sw_ooxmlexport_libraries) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_roundtrip_diff,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_roundtrip_diff,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/source/uibase/inc \
	-I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_roundtrip_diff,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_roundtrip_diff))
$(eval $(call gb_CppunitTest_use_vcl,sw_roundtrip_diff))

$(eval $(call gb_CppunitTest_use_rdb,sw_roundtrip_diff,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_roundtrip_diff))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_roundtrip_diff,\
    modules/swriter \
    sfx \
    svt \
))

$(eval $(call gb_CppunitTest_use_packages,sw_roundtrip_diff,\
	oox_customshapes \
	oox_generated \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_roundtrip_diff))

# vim: set noet sw=4 ts=4:
