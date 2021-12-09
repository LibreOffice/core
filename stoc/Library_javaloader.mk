# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,javaloader))

$(eval $(call gb_Library_use_external,javaloader,boost_headers))

$(eval $(call gb_Library_use_udk_api,javaloader))

$(eval $(call gb_Library_use_libraries,javaloader,\
    cppu \
    cppuhelper \
    jvmaccess \
    sal \
    salhelper \
    tl \
))

$(eval $(call gb_Library_set_componentfile,javaloader,stoc/source/javaloader/javaloader,ure/services))

$(eval $(call gb_Library_add_exception_objects,javaloader,\
    stoc/source/javaloader/javaloader \
))

# vim:set noet sw=4 ts=4:
