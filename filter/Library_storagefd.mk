# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_Library_Library,storagefd))

$(eval $(call gb_Library_set_componentfile,storagefd,filter/source/storagefilterdetect/storagefd,services))

$(eval $(call gb_Library_use_external,storagefd,boost_headers))

$(eval $(call gb_Library_use_sdk_api,storagefd))

$(eval $(call gb_Library_use_libraries,storagefd,\
    comphelper \
    cppuhelper \
    cppu \
    sal \
    sfx \
    tl \
    utl \
))

$(eval $(call gb_Library_add_exception_objects,storagefd,\
    filter/source/storagefilterdetect/filterdetect \
))

# vim: set noet sw=4 ts=4:
