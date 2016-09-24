# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl_textenc))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl_textenc,\
    sal/qa/rtl/textenc/rtl_tencinfo \
    sal/qa/rtl/textenc/rtl_textcvt \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl_textenc,\
    sal \
    $(gb_UWINAPI) \
))

# The test uses O(U)String capabilities that dlopen this lib
$(call gb_CppunitTest_get_target,sal_rtl_textenc) : \
    $(call gb_Library_get_target,sal_textenc)

# vim: set noet sw=4 ts=4:
