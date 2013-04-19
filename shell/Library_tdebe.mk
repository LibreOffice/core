# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,tdebe1))

$(eval $(call gb_Library_use_packages,tdebe1,\
))

$(eval $(call gb_Library_use_sdk_api,tdebe1))

$(eval $(call gb_Library_use_externals,tdebe1,\
	boost_headers \
	tde \
))

$(eval $(call gb_Library_use_libraries,tdebe1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,tdebe1,shell/source/backends/kdebe/tdebe1))

$(eval $(call gb_Library_add_exception_objects,tdebe1,\
    shell/source/backends/kdebe/kdeaccess \
    shell/source/backends/kdebe/kdebackend \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
