# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,unotest))

$(eval $(call gb_Library_add_defs,unotest,\
	-DOOO_DLLIMPLEMENTATION_UNOTEST \
))

$(eval $(call gb_Library_use_sdk_api,unotest))

$(eval $(call gb_Library_use_libraries,unotest,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,unotest,\
	boost_headers \
	cppunit \
))

$(eval $(call gb_Library_add_exception_objects,unotest,\
    unotest/source/cpp/bootstrapfixturebase \
    unotest/source/cpp/filters-test \
    unotest/source/cpp/getargument \
    unotest/source/cpp/gettestargument \
    unotest/source/cpp/macros_test \
    unotest/source/cpp/officeconnection \
    unotest/source/cpp/toabsolutefileurl \
    unotest/source/cpp/uniquepipename \
))

# vim: set noet sw=4 ts=4:
