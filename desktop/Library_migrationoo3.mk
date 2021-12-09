# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,migrationoo3))

$(eval $(call gb_Library_use_sdk_api,migrationoo3))

$(eval $(call gb_Library_use_libraries,migrationoo3,\
    cppu \
    cppuhelper \
    sal \
    tl \
    utl \
))

$(eval $(call gb_Library_set_componentfile,migrationoo3,desktop/source/migration/services/migrationoo3,services))

$(eval $(call gb_Library_add_exception_objects,migrationoo3,\
    desktop/source/migration/services/oo3extensionmigration \
))

# vim: set ts=4 sw=4 et:
