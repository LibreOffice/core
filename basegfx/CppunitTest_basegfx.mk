# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,basegfx_cpputest))

$(eval $(call gb_CppunitTest_use_api,basegfx_cpputest,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,basegfx_cpputest,\
    basegfx \
    cppu \
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,basegfx_cpputest,\
    basegfx/test/basegfx1d \
    basegfx/test/basegfx2d \
    basegfx/test/basegfx3d \
    basegfx/test/boxclipper \
    basegfx/test/basegfxtools \
    basegfx/test/clipstate \
    basegfx/test/genericclipper \
))

# vim: set noet sw=4 ts=4:
