# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,unobootstrapprotector))

$(eval $(call gb_Library_use_sdk_api,unobootstrapprotector))

$(eval $(call gb_Library_use_libraries,unobootstrapprotector,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,unobootstrapprotector,\
	boost_headers \
	cppunit \
))

$(eval $(call gb_Library_add_exception_objects,unobootstrapprotector,\
	unotest/source/cpp/unobootstrapprotector/unobootstrapprotector \
))

# runtime dependency: needs unorc/uno.ini
$(eval $(call gb_Library_use_package,unobootstrapprotector,\
	instsetoo_native_setup_ure \
))

# vim: set noet sw=4 ts=4:
