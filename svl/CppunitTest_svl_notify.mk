# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svl_notify))

$(eval $(call gb_CppunitTest_use_external,svl_notify,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,svl_notify))

$(eval $(call gb_CppunitTest_add_exception_objects,svl_notify, \
	svl/qa/unit/notify/test_SfxBroadcaster \
))

$(eval $(call gb_CppunitTest_use_libraries,svl_notify, \
	svl \
	comphelper \
	sal \
	cppu \
	cppuhelper \
))

$(eval $(call gb_CppunitTest_set_include,svl_notify,\
	-I$(SRCDIR)/svl/source/inc \
	$$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
