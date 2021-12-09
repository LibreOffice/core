# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,ucpimage))

$(eval $(call gb_Library_add_exception_objects,ucpimage, \
    ucb/source/ucp/image/ucpimage \
))

$(eval $(call gb_Library_set_componentfile,ucpimage,ucb/source/ucp/image/ucpimage,services))

$(eval $(call gb_Library_use_externals,ucpimage, \
    boost_headers \
))

$(eval $(call gb_Library_use_libraries,ucpimage, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    ucbhelper \
    vcl \
))

$(eval $(call gb_Library_use_sdk_api,ucpimage))

# vim: set noet sw=4 ts=4:
