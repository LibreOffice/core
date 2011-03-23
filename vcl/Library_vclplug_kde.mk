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

$(eval $(call gb_Library_Library,vclplug_kde))

$(eval $(call gb_Library_set_include,vclplug_kde,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_cxxflags,vclplug_kde,\
    $$(CXXFLAGS) \
    $$(KDE_CFLAGS) \
))

$(eval $(call gb_Library_set_defs,vclplug_kde,\
    $$(DEFS) \
    -DVCLPLUG_KDE_IMPLEMENTATION \
))

$(eval $(call gb_Library_set_ldflags,vclplug_kde,\
    $$(KDE_LIBS)\
    $$(LDFLAGS) \
))

$(eval $(call gb_Library_add_linked_libs,vclplug_kde,\
    vclplug_gen \
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    icuuc \
    icule \
    i18nisolang1 \
    i18npaper \
    i18nutil \
    jvmaccess \
    stl \
    cppu \
    sal \
    vos3 \
    X11 \
    Xext \
    SM \
    ICE \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_kde,\
    vcl/unx/kde/kdedata \
    vcl/unx/kde/salnativewidgets-kde \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vclplug_kde,\
    dl \
    m \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
