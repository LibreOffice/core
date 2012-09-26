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

$(eval $(call gb_Library_Library,vclplug_kde4))

$(eval $(call gb_Library_use_custom_headers,vclplug_kde4,vcl/unx/kde4))

$(eval $(call gb_Library_set_include,vclplug_kde4,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,vclplug_kde4,\
    -DVCLPLUG_KDE4_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_kde4))

$(eval $(call gb_Library_use_libraries,vclplug_kde4,\
    vclplug_gen \
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

$(eval $(call gb_Library_add_standard_system_libs,vclplug_kde4))

$(eval $(call gb_Library_use_externals,vclplug_kde4,\
	icule \
	icuuc \
	kde4 \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_kde4,\
    vcl/unx/kde4/KDEData \
	vcl/unx/kde4/KDE4FilePicker \
    vcl/unx/kde4/KDESalDisplay \
    vcl/unx/kde4/KDESalFrame \
    vcl/unx/kde4/KDESalGraphics \
    vcl/unx/kde4/KDESalInstance \
    vcl/unx/kde4/KDEXLib \
    vcl/unx/kde4/main \
    vcl/unx/kde4/VCLKDEApplication \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_use_libraries,vclplug_kde4,\
    dl \
    m \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
