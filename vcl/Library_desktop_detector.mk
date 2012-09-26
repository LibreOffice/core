# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,desktop_detector))

#$(eval $(call gb_Library_use_package,vcl,vcl_inc))

$(eval $(call gb_Library_set_include,desktop_detector,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,desktop_detector,\
    -DDESKTOP_DETECTOR_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,desktop_detector))

$(eval $(call gb_Library_use_libraries,desktop_detector,\
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nisolang1 \
    i18nutil \
    jvmaccess \
    cppu \
    sal \
    X11 \
    Xext \
    SM \
    ICE \
))

$(eval $(call gb_Library_add_standard_system_libs,desktop_detector))

$(eval $(call gb_Library_use_externals,desktop_detector,\
	icule \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,desktop_detector,\
    vcl/unx/generic/desktopdetect/desktopdetector \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_use_libraries,desktop_detector,\
    dl \
    m \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
