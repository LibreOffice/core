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

$(eval $(call gb_Library_Library,vclplug_svp))

$(eval $(call gb_Library_set_include,vclplug_svp,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_svp,\
    $$(FREETYPE_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_svp,\
    -DVCLPLUG_SVP_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_svp))

$(eval $(call gb_Library_use_libraries,vclplug_svp,\
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    basebmp \
    comphelper \
    cppuhelper \
    i18nisolang1 \
    i18nutil \
    jvmaccess \
    cppu \
    sal \
))

$(eval $(call gb_Library_add_standard_system_libs,vclplug_svp))

$(eval $(call gb_Library_use_externals,vclplug_svp,\
	icule \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_svp,\
    vcl/headless/svpgdi \
    vcl/headless/svpinst \
    vcl/headless/svpdata \
))

$(eval $(call gb_Library_use_static_libraries,vclplug_svp,\
    headless \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_use_libraries,vclplug_svp,\
    dl \
    m \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
