# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,active_native))

$(eval $(call gb_Library_add_exception_objects,active_native, \
    desktop/test/deployment/active/active_native \
))

$(eval $(call gb_Library_use_externals,active_native, \
    boost_headers \
))

$(eval $(call gb_Library_use_libraries,active_native, \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_sdk_api,active_native))

# vim: set noet sw=4 ts=4:
