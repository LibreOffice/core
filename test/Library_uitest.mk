# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,uitest))

$(eval $(call gb_Library_add_defs,uitest,\
    -DOOO_DLLIMPLEMENTATION_UITEST \
))

$(eval $(call gb_Library_use_sdk_api,uitest))

$(eval $(call gb_Library_use_externals,uitest,\
    boost_headers \
    cppunit \
    libxml2 \
))

$(eval $(call gb_Library_use_libraries,uitest,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    i18nlangtag \
    sal \
    svt \
    tl \
    unotest \
    utl \
    vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,uitest,\
    test/source/uitest/uitest \
    test/source/uitest/uiobject \
))

# vim: set noet sw=4 ts=4:
