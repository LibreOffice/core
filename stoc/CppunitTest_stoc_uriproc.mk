# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,stoc_uriproc))

$(eval $(call gb_CppunitTest_add_exception_objects,stoc_uriproc, \
    stoc/test/uriproc/test_uriproc \
))

$(eval $(call gb_CppunitTest_use_libraries,stoc_uriproc, \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_CppunitTest_use_udk_api,stoc_uriproc))

$(eval $(call gb_CppunitTest_use_ure,stoc_uriproc))

# vim: set noet sw=4 ts=4:
