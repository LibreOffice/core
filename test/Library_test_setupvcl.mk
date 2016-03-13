# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,test-setupvcl))

$(eval $(call gb_Library_add_defs,test-setupvcl,\
    -DOOO_DLLIMPLEMENTATION_TEST_SETUPVCL \
))

$(eval $(call gb_Library_add_exception_objects,test-setupvcl, \
    test/source/setupvcl \
))

$(eval $(call gb_Library_use_externals,test-setupvcl, \
    boost_headers \
))

$(eval $(call gb_Library_use_libraries,test-setupvcl, \
    comphelper \
    cppu \
    i18nlangtag \
    sal \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_use_sdk_api,test-setupvcl))

# vim: set noet sw=4 ts=4:
