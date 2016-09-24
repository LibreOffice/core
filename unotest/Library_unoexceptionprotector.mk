# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,unoexceptionprotector))

$(eval $(call gb_Library_use_sdk_api,unoexceptionprotector))

$(eval $(call gb_Library_use_libraries,unoexceptionprotector,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,unoexceptionprotector,\
	boost_headers \
	cppunit \
))

$(eval $(call gb_Library_add_exception_objects,unoexceptionprotector,\
	unotest/source/cpp/unoexceptionprotector/unoexceptionprotector \
))

# vim: set noet sw=4 ts=4:
