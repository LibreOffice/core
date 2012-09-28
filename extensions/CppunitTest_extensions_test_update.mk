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
# Copyright (C) 2011 Jan Holesovsky <kendy@suse.cz> (initial developer)
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
#

$(eval $(call gb_CppunitTest_CppunitTest,extensions_test_update))

$(eval $(call gb_CppunitTest_add_exception_objects,extensions_test_update, \
	extensions/qa/update/test_update \
))

$(eval $(call gb_CppunitTest_use_libraries,extensions_test_update, \
	updchk \
	cppu \
	cppuhelper \
	sal \
	test \
	unotest \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,extensions_test_update,\
	shell32 \
	ole32 \
))
endif

$(eval $(call gb_CppunitTest_use_external,extensions_test_update,curl))

$(eval $(call gb_CppunitTest_set_include,extensions_test_update,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_CppunitTest_use_api,extensions_test_update,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,extensions_test_update))

$(eval $(call gb_CppunitTest_use_components,extensions_test_update,\
    configmgr/source/configmgr \
    extensions/source/update/feed/updatefeed \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,extensions_test_update))

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:
