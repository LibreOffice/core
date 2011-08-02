# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mono_loader))

$(eval $(call gb_Library_add_cxxflags,mono_loader,\
        $(MONO_CFLAGS) \
))

$(eval $(call gb_Library_use_udk_api,mono_loader))

$(eval $(call gb_Library_add_ldflags,mono_loader,\
        $(MONO_LIBS) \
))

$(eval $(call gb_Library_use_libraries,mono_loader,\
	sal \
	cppu \
	cppuhelper \
))

$(eval $(call gb_Library_add_exception_objects,mono_loader,\
    cli_ure/source/mono_loader/service \
    cli_ure/source/mono_loader/mono_loader \
))

# vim: set noet sw=4 ts=4:
