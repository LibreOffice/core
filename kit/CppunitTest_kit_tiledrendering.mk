# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,kit_tiledrendering))

$(eval $(call gb_CppunitTest_add_exception_objects,kit_tiledrendering, \
    kit/qa/unit/tiledrendering \
))

$(eval $(call gb_CppunitTest_use_external,kit_tiledrendering,boost_headers))

# We need all these libraries / etc. due for CppunitTest to work, even though
# our test specifically tests COKit only functionality which would otherwise not
# require any normal LO api/libraries.
$(eval $(call gb_CppunitTest_use_libraries,kit_tiledrendering, \
	sal \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,kit_tiledrendering,\
    -lm \
    -ldl \
))
endif

$(eval $(call gb_CppunitTest_use_api,kit_tiledrendering,\
    offapi \
))

$(eval $(call gb_CppunitTest_use_configuration,kit_tiledrendering))

# vim: set noet sw=4 ts=4:
