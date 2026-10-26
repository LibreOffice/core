# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_complex))

$(eval $(call gb_CppunitTest_use_externals,sfx2_complex,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_complex, \
    sfx2/qa/complex/UndoManager \
))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_complex, \
    comphelper \
    cppu \
    cppuhelper \
    embobj \
	fwk \
    sal \
    salhelper \
    sfx \
    subsequenttest \
    svl \
    test \
	tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sfx2_complex))

$(eval $(call gb_CppunitTest_use_ure,sfx2_complex))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_complex))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_complex,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sfx2_complex,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sfx2_complex))

$(eval $(call gb_CppunitTest_use_uiconfigs,sfx2_complex,\
    cui \
    dbaccess \
    formula \
    filter \
    fps \
    modules/schart \
    modules/scalc \
    modules/sdraw \
    modules/smath \
    modules/swriter \
    modules/simpress \
    sfx \
    svt \
    svx \
    vcl \
))

# vim: set noet sw=4 ts=4:
