# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,macbe1))

$(eval $(call gb_Library_use_external,macbe1,boost_headers))

$(eval $(call gb_Library_use_sdk_api,macbe1))

$(eval $(call gb_Library_use_system_darwin_frameworks,macbe1,\
	Cocoa \
	SystemConfiguration \
))

$(eval $(call gb_Library_use_libraries,macbe1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,macbe1,shell/source/backends/macbe/macbe1,services))

$(eval $(call gb_Library_add_objcxxobjects,macbe1,\
    shell/source/backends/macbe/macbackend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
