# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,offacc))

$(eval $(call gb_Library_use_sdk_api,offacc))

$(eval $(call gb_Library_use_libraries,offacc,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
))

$(eval $(call gb_Library_set_componentfile,offacc,desktop/source/offacc/offacc,services))

$(eval $(call gb_Library_add_exception_objects,offacc,\
    desktop/source/offacc/acceptor \
))

# vim: set ts=4 sw=4 et:
