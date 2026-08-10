# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scriptinterop))

$(eval $(call gb_Library_set_componentfile,scriptinterop,scriptinterop/util/scriptinterop,services))

$(eval $(call gb_Library_add_exception_objects,scriptinterop,\
    scriptinterop/source/cool \
    scriptinterop/source/document \
    scriptinterop/source/drawing \
    scriptinterop/source/presentation \
    scriptinterop/source/spreadsheet \
))

$(eval $(call gb_Library_use_libraries,scriptinterop,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_api,scriptinterop,\
    udkapi \
    offapi \
    scriptinterop \
))

# vim: set noet sw=4 ts=4:
