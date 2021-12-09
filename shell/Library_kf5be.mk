# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kf5be1))

$(eval $(call gb_Library_use_sdk_api,kf5be1))

$(eval $(call gb_Library_use_externals,kf5be1,\
	boost_headers \
	kf5 \
))

$(eval $(call gb_Library_use_libraries,kf5be1,\
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_Library_set_componentfile,kf5be1,shell/source/backends/kf5be/kf5be1,services))

$(eval $(call gb_Library_add_exception_objects,kf5be1,\
    shell/source/backends/kf5be/kf5access \
    shell/source/backends/kf5be/kf5backend \
))

# Workaround for clang+icecream (clang's -frewrite-includes
# doesn't handle Qt5's QT_HAS_INCLUDE that Qt5 uses for <chrono>).
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_Library_add_cxxflags,kf5be1, \
    -include chrono \
))
endif

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
