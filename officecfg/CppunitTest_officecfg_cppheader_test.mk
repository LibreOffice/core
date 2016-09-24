# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,officecfg_cppheader_test))

$(eval $(call gb_CppunitTest_use_libraries,officecfg_cppheader_test,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_custom_headers,officecfg_cppheader_test,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_add_exception_objects,officecfg_cppheader_test, \
    officecfg/qa/cppheader \
))

$(eval $(call gb_CppunitTest_use_api,officecfg_cppheader_test, \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_externals,officecfg_cppheader_test, \
    boost_headers \
))

# vim: set noet sw=4 ts=4:
