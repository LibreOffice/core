# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kde4be1))

$(eval $(call gb_Library_use_sdk_api,kde4be1))

$(eval $(call gb_Library_use_externals,kde4be1,\
	boost_headers \
	kde4 \
))

$(eval $(call gb_Library_use_libraries,kde4be1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,kde4be1,shell/source/backends/kde4be/kde4be1))

$(eval $(call gb_Library_add_exception_objects,kde4be1,\
    shell/source/backends/kde4be/kde4access \
    shell/source/backends/kde4be/kde4backend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
