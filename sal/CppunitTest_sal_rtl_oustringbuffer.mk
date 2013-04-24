# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl_oustringbuffer))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl_oustringbuffer,\
    sal/qa/rtl/oustringbuffer/test_oustringbuffer_appendchar \
    sal/qa/rtl/oustringbuffer/test_oustringbuffer_noadditional \
    sal/qa/rtl/oustringbuffer/test_oustringbuffer_tostring \
    sal/qa/rtl/oustringbuffer/test_oustringbuffer_utf32 \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl_oustringbuffer,\
    sal \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
