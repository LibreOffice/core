# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,gconfbe1))

ifeq ($(ENABLE_LOCKDOWN),TRUE)
$(eval $(call gb_Library_add_defs,gconfbe1,\
	-DENABLE_LOCKDOWN \
))
endif

$(eval $(call gb_Library_use_sdk_api,gconfbe1))

$(eval $(call gb_Library_use_externals,gconfbe1,\
	boost_headers \
	gconf \
))

$(eval $(call gb_Library_use_libraries,gconfbe1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,gconfbe1,shell/source/backends/gconfbe/gconfbe1))

$(eval $(call gb_Library_add_exception_objects,gconfbe1,\
    shell/source/backends/gconfbe/gconfaccess \
    shell/source/backends/gconfbe/gconfbackend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
