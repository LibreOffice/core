# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,jsuno))

$(eval $(call gb_Library_add_defs,jsuno,-DLO_DLLIMPLEMENTATION_JSUNO))

$(eval $(call gb_Library_add_exception_objects,jsuno, \
    jsuno/source/jsuno \
))

$(eval $(call gb_Library_use_externals,jsuno, \
    qjs \
))

$(eval $(call gb_Library_use_libraries,jsuno, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_sdk_api,jsuno))

# vim: set noet sw=4 ts=4:
