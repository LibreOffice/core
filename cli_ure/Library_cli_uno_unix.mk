# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,cli_uno))

$(eval $(call gb_Library_add_cxxflags,cli_uno,\
        $(MONO_CFLAGS) \
        -fvisibility=default \
))

$(eval $(call gb_Library_use_udk_api,cli_uno))

$(eval $(call gb_Library_add_ldflags,cli_uno,\
        $(MONO_LIBS) \
))

$(eval $(call gb_Library_use_libraries,cli_uno,\
	sal \
	cppu \
	cppuhelper \
))

$(eval $(call gb_Library_add_exception_objects,cli_uno,\
    cli_ure/source/mono_bridge/mono_bridge \
    cli_ure/source/mono_bridge/mono_proxy \
    cli_ure/source/mono_bridge/uno_glue \
))

# vim: set noet sw=4 ts=4:
