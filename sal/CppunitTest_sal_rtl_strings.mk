# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl_strings))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl_strings,\
    sal/qa/rtl/strings/test_strings_replace \
    sal/qa/rtl/strings/test_ostring \
    sal/qa/rtl/strings/test_ostring_concat \
    sal/qa/rtl/strings/test_ostring_stringliterals \
    sal/qa/rtl/strings/test_oustring_compare \
    sal/qa/rtl/strings/test_oustring_concat \
    sal/qa/rtl/strings/test_oustring_convert \
    sal/qa/rtl/strings/test_oustring_endswith \
    sal/qa/rtl/strings/test_oustring_noadditional \
    sal/qa/rtl/strings/test_oustring_startswith \
    sal/qa/rtl/strings/test_oustring_stringliterals \
    sal/qa/rtl/strings/test_strings_toint \
    sal/qa/rtl/strings/test_strings_valuex \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl_strings,\
    sal \
	$(gb_UWINAPI) \
))

# The test uses O(U)String capabilities that dlopen this lib
$(call gb_CppunitTest_get_target,sal_rtl_strings) : \
    $(call gb_Library_get_target,sal_textenc)

# vim: set noet sw=4 ts=4:
