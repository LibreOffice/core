# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# this is a Library that contains a unit test, so it can be packaged
$(eval $(call gb_Library_Library,smoketest))

# necessary because this is no CppUnitTest but a Library
$(eval $(call gb_Library_add_defs,smoketest,\
	-DCPPUNIT_PLUGIN_EXPORT='extern "C" SAL_DLLPUBLIC_EXPORT' \
))

$(eval $(call gb_Library_use_sdk_api,smoketest))

$(eval $(call gb_Library_use_libraries,smoketest,\
	cppu \
	cppuhelper \
	sal \
	unotest \
))

$(eval $(call gb_Library_use_externals,smoketest,\
	boost_headers \
	cppunit \
))

$(eval $(call gb_Library_add_exception_objects,smoketest,\
	smoketest/smoketest \
))

# vim: set noet sw=4 ts=4:
