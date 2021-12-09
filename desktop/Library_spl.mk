# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,spl))

$(eval $(call gb_Library_set_include,spl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
))

$(eval $(call gb_Library_use_external,spl,boost_headers))

$(eval $(call gb_Library_use_sdk_api,spl))

$(eval $(call gb_Library_use_libraries,spl,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    utl \
    vcl \
))

$(eval $(call gb_Library_set_componentfile,spl,desktop/source/splash/spl,services))

$(eval $(call gb_Library_add_exception_objects,spl,\
    desktop/source/splash/splash \
    desktop/source/splash/unxsplash \
))

# vim: set ts=4 sw=4 et:
