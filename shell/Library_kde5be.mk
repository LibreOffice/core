# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kde5be1))

$(eval $(call gb_Library_use_sdk_api,kde5be1))

$(eval $(call gb_Library_use_externals,kde5be1,\
	boost_headers \
	kde5 \
))

$(eval $(call gb_Library_use_libraries,kde5be1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,kde5be1,shell/source/backends/kde5be/kde5be1))

$(eval $(call gb_Library_add_exception_objects,kde5be1,\
    shell/source/backends/kde5be/kde5access \
    shell/source/backends/kde5be/kde5backend \
))

# Workaround for clang+icecream (clang's -frewrite-includes
# doesn't handle Qt5's QT_HAS_INCLUDE that Qt5 uses for <chrono>).
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_Library_add_defs,kde5be1, \
    -include chrono \
))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
