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

$(eval $(call gb_Library_Library,vclplug_gtk3))

$(eval $(call gb_Library_set_warnings_not_errors,vclplug_gtk3))

$(eval $(call gb_Library_set_include,vclplug_gtk3,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/unx \
    -I$(SRCDIR)/vcl/unx/gtk3/inc \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_gtk3,\
    $$(INCLUDE) \
    $$(GTK3_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_gtk3,\
    -DVCLPLUG_GTK_IMPLEMENTATION \
    -DLIBO_VERSION=\"$(UPD)$(LAST_MINOR)\" \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gtk3))

$(eval $(call gb_Library_add_libs,vclplug_gtk3,\
    $$(GTK3_LIBS) \
    $$(GTHREAD_LIBS) \
))

$(eval $(call gb_Library_use_libraries,vclplug_gtk3,\
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

$(eval $(call gb_Library_add_standard_system_libs,vclplug_gtk3))

$(eval $(call gb_Library_use_externals,vclplug_gtk3,\
	dbus \
))

$(eval $(call gb_Library_use_libraries,vclplug_gtk3,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_gtk3,\
    vcl/unx/gtk3/a11y/gtk3atkaction \
    vcl/unx/gtk3/a11y/gtk3atkbridge \
    vcl/unx/gtk3/a11y/gtk3atkcomponent \
    vcl/unx/gtk3/a11y/gtk3atkeditabletext \
    vcl/unx/gtk3/a11y/gtk3atkfactory \
    vcl/unx/gtk3/a11y/gtk3atkhypertext \
    vcl/unx/gtk3/a11y/gtk3atkimage \
    vcl/unx/gtk3/a11y/gtk3atklistener \
    vcl/unx/gtk3/a11y/gtk3atkregistry \
    vcl/unx/gtk3/a11y/gtk3atkselection \
    vcl/unx/gtk3/a11y/gtk3atktable \
    vcl/unx/gtk3/a11y/gtk3atktextattributes \
    vcl/unx/gtk3/a11y/gtk3atktext \
    vcl/unx/gtk3/a11y/gtk3atkutil \
    vcl/unx/gtk3/a11y/gtk3atkvalue \
    vcl/unx/gtk3/a11y/gtk3atkwindow \
    vcl/unx/gtk3/a11y/gtk3atkwrapper \
    vcl/unx/gtk3/app/gtk3gtkdata \
    vcl/unx/gtk3/app/gtk3gtkinst \
    vcl/unx/gtk3/app/gtk3gtksys \
    vcl/unx/gtk3/app/gtk3fpicker \
    vcl/unx/gtk3/gdi/gtk3gtkprintwrapper \
    vcl/unx/gtk3/gdi/gtk3salnativewidgets-gtk \
    vcl/unx/gtk3/gdi/gtk3salprn-gtk \
    vcl/unx/gtk3/window/gtk3gtkframe \
    vcl/unx/gtk3/window/gtk3gtkobject \
))

$(eval $(call gb_Library_use_static_libraries,vclplug_gtk3,\
    headless \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gtk3,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
