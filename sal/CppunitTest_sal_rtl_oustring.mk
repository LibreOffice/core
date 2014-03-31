# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl_oustring))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl_oustring,\
    sal/qa/rtl/oustring/rtl_OUString2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl_oustring,\
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sal_rtl_oustring,\
		    -I$(SRCDIR)/sal/qa/inc \
			    $$(INCLUDE) \
				))

# The test uses O(U)String capabilities that dlopen this lib:
$(call gb_CppunitTest_get_target,sal_rtl_oustring) : \
    $(call gb_Library_get_target,sal_textenc)

# vim: set noet sw=4 ts=4:
