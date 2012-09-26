# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl_strings))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl_strings,\
    sal/qa/rtl/strings/test_strings_replace \
    sal/qa/rtl/strings/test_ostring_stringliterals \
    sal/qa/rtl/strings/test_oustring_compare \
    sal/qa/rtl/strings/test_oustring_convert \
    sal/qa/rtl/strings/test_oustring_endswith \
    sal/qa/rtl/strings/test_oustring_noadditional \
    sal/qa/rtl/strings/test_oustring_startswith \
    sal/qa/rtl/strings/test_oustring_stringliterals \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl_strings,\
    sal \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

# The test uses O(U)String capabilities that dlopen this lib
ifneq ($(OS),ANDROID)
# Except that on Android we don't build it separately
$(call gb_CppunitTest_get_target,sal_rtl_strings) : \
    $(call gb_Library_get_target,sal_textenc)
endif

# vim: set noet sw=4 ts=4:
