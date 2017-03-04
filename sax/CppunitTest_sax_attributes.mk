# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sax_attributes))

$(eval $(call gb_CppunitTest_add_exception_objects,sax_attributes, \
	sax/qa/cppunit/attributes \
))

$(eval $(call gb_CppunitTest_use_libraries,sax_attributes, \
	cppu \
	cppuhelper \
	sal \
	sax \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sax_attributes))

$(eval $(call gb_CppunitTest_use_ure,sax_attributes))

# vim: set noet sw=4 ts=4:
