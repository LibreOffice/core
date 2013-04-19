# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kdebe1))

$(eval $(call gb_Library_use_packages,kdebe1,\
	shell_inc \
))

$(eval $(call gb_Library_add_libs,kdebe1,\
	-lkio \
))

$(eval $(call gb_Library_use_sdk_api,kdebe1))

$(eval $(call gb_Library_use_externals,kdebe1,\
	boost_headers \
	kde \
))

$(eval $(call gb_Library_use_libraries,kdebe1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,kdebe1,shell/source/backends/kdebe/kdebe1))

$(eval $(call gb_Library_add_exception_objects,kdebe1,\
    shell/source/backends/kdebe/kdeaccess \
    shell/source/backends/kdebe/kdebackend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
