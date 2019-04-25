# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,libreofficekit_tiledrendering))

$(eval $(call gb_CppunitTest_add_exception_objects,libreofficekit_tiledrendering, \
    libreofficekit/qa/unit/tiledrendering \
))

$(eval $(call gb_CppunitTest_use_external,libreofficekit_tiledrendering,boost_headers))

# We need all these libraries / etc. due for CppunitTest to work, even though
# our test specifically tests LOK only functionality which would otherwise not
# require any normal LO api/libraries.
$(eval $(call gb_CppunitTest_use_libraries,libreofficekit_tiledrendering, \
	sal \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,libreofficekit_tiledrendering,\
    -lm \
    -ldl \
))
endif

$(eval $(call gb_CppunitTest_use_api,libreofficekit_tiledrendering,\
    offapi \
))

$(eval $(call gb_CppunitTest_use_configuration,libreofficekit_tiledrendering))

# vim: set noet sw=4 ts=4:
