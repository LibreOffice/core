# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,desktopbe1))

$(eval $(call gb_Library_use_external,desktopbe1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,desktopbe1))

$(eval $(call gb_Library_use_libraries,desktopbe1,\
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
))

$(eval $(call gb_Library_set_componentfile,desktopbe1,shell/source/backends/desktopbe/desktopbe1,services))

$(eval $(call gb_Library_add_exception_objects,desktopbe1,\
    shell/source/backends/desktopbe/desktopbackend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
