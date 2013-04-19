# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,spl_unx))

$(eval $(call gb_Library_use_sdk_api,spl_unx))

$(eval $(call gb_Library_use_libraries,spl_unx,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_set_componentfile,spl_unx,desktop/unx/splash/splash))

$(eval $(call gb_Library_add_exception_objects,spl_unx,\
    desktop/unx/splash/unxsplash \
))

# vim: set ts=4 sw=4 et:
