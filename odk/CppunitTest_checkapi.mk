# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,odk_checkapi))

$(eval $(call gb_CppunitTest_use_custom_headers,odk_checkapi,\
	odk/allheaders \
))

$(eval $(call gb_CppunitTest_add_exception_objects,odk_checkapi,\
    odk/qa/checkapi/strings \
))

$(eval $(call gb_CppunitTest_use_external,odk_checkapi,boost_headers))

$(eval $(call gb_CppunitTest_use_internal_comprehensive_api,odk_checkapi,\
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,odk_checkapi,\
	cppu \
	cppuhelper \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
