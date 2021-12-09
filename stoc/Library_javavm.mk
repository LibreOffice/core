# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,javavm))

$(eval $(call gb_Library_use_external,javavm,boost_headers))

$(eval $(call gb_Library_use_udk_api,javavm))

$(eval $(call gb_Library_use_libraries,javavm,\
    cppu \
    cppuhelper \
    i18nlangtag \
    jvmaccess \
    jvmfwk \
    sal \
    salhelper \
    tl \
))

$(eval $(call gb_Library_set_componentfile,javavm,stoc/source/javavm/javavm,ure/services))

$(eval $(call gb_Library_add_exception_objects,javavm,\
    stoc/source/javavm/interact \
    stoc/source/javavm/javavm \
    stoc/source/javavm/jvmargs \
))

# vim:set noet sw=4 ts=4:
