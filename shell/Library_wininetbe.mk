# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,wininetbe1))

$(eval $(call gb_Library_use_sdk_api,wininetbe1))

$(eval $(call gb_Library_use_libraries,wininetbe1,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,wininetbe1,shell/source/backends/wininetbe/wininetbe1))

$(eval $(call gb_Library_add_exception_objects,wininetbe1,\
    shell/source/backends/wininetbe/wininetbackend \
    shell/source/backends/wininetbe/wininetbecdef \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
