# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,cppuhelper_cppu_ifcontainer))

$(eval $(call gb_CppunitTest_add_exception_objects,cppuhelper_cppu_ifcontainer,\
    cppuhelper/qa/ifcontainer/cppu_ifcontainer \
))

$(eval $(call gb_CppunitTest_use_api,cppuhelper_cppu_ifcontainer,\
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,cppuhelper_cppu_ifcontainer,\
	cppu \
    cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

# vim: set noet sw=4 ts=4:
