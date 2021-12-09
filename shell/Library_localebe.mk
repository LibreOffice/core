# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,localebe1))

$(eval $(call gb_Library_use_sdk_api,localebe1))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,localebe1,\
	CoreFoundation \
	CoreServices \
))
endif

$(eval $(call gb_Library_use_libraries,localebe1,\
	cppu \
	cppuhelper \
	sal \
	i18nlangtag \
))

$(eval $(call gb_Library_set_componentfile,localebe1,shell/source/backends/localebe/localebe1,services))

$(eval $(call gb_Library_add_exception_objects,localebe1,\
    shell/source/backends/localebe/localebackend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
