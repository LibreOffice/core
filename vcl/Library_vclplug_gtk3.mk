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

$(eval $(call gb_Library_set_include,vclplug_gtk3,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_cxxflags,vclplug_gtk3,\
    $$(CXXFLAGS) \
    $$(GTK3_CFLAGS) \
))

$(eval $(call gb_Library_set_defs,vclplug_gtk3,\
    $$(DEFS) \
    -DVCLPLUG_GTK_IMPLEMENTATION \
    -DVERSION=\"$(UPD)$(LAST_MINOR)\" \
))

ifneq ($(ENABLE_DBUS),)
$(eval $(call gb_Library_set_include,vclplug_gtk3,\
	$$(INCLUDE) \
	$(filter -I%,$(shell pkg-config --cflags dbus-glib-1)) \
))
$(eval $(call gb_Library_set_defs,vclplug_gtk3,\
    $$(DEFS) \
    -DENABLE_DBUS \
))
$(eval $(call gb_Library_set_ldflags,vclplug_gtk3,\
    $$(LDFLAGS) \
    $(shell pkg-config --libs dbus-glib-1)\
))
endif

$(eval $(call gb_Library_set_ldflags,vclplug_gtk3,\
    $$(LDFLAGS) \
    $$(GTK3_LIBS) \
    $$(GTHREAD_LIBS) \
))

$(eval $(call gb_Library_add_linked_libs,vclplug_gtk3,\
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
    cppu \
    sal \
    X11 \
    Xext \
    SM \
    ICE \
    $(gb_STDLIBS) \
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
    vcl/unx/gtk3/window/gtk3gtkframe \
    vcl/unx/gtk3/window/gtk3gtkobject \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vclplug_gtk3,\
    dl \
    m \
    pthread \
))
endif
# vim: set noet sw=4 ts=4:
