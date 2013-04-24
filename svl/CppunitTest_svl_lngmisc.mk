# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Copyright (C) 2011 August Sodora <augsod@gmail.com>
#

$(eval $(call gb_CppunitTest_CppunitTest,svl_lngmisc))

$(eval $(call gb_CppunitTest_add_exception_objects,svl_lngmisc, \
svl/qa/unit/test_lngmisc \
))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_use_libraries,svl_lngmisc, \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    svl \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,svl_lngmisc, \
	oleaut32 \
))
endif

$(eval $(call gb_CppunitTest_set_include,svl_lngmisc,\
	-I$(SRCDIR)/svl/source/inc \
	$$(INCLUDE) \
))
# vim: set noet sw=4 ts=4:
