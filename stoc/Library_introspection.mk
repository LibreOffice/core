# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,introspection))

$(eval $(call gb_Library_use_external,introspection,boost_headers))

$(eval $(call gb_Library_use_udk_api,introspection))

$(eval $(call gb_Library_use_libraries,introspection,\
    cppu \
    cppuhelper \
    sal \
    salhelper \
))

$(eval $(call gb_Library_set_componentfile,introspection,stoc/source/inspect/introspection,ure/services))

$(eval $(call gb_Library_add_exception_objects,introspection,\
    stoc/source/inspect/introspection \
))

# vim:set noet sw=4 ts=4:
