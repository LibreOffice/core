# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,vclplug_gtk))

$(eval $(call gb_Library_set_include,vclplug_gtk,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/unx \
    -I$(SRCDIR)/vcl/unx/gtk/inc \
))

$(eval $(call gb_Library_add_defs,vclplug_gtk,\
    -DVCLPLUG_GTK_IMPLEMENTATION \
))

ifeq ($(ENABLE_DBUS),TRUE)
$(eval $(call gb_Library_add_defs,vclplug_gtk,\
	-DENABLE_DBUS \
))
endif

$(eval $(call gb_Library_use_custom_headers,vclplug_gtk,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gtk))

$(eval $(call gb_Library_use_libraries,vclplug_gtk,\
    vclplug_gen \
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(ENABLE_JAVA), \
        jvmaccess) \
    cppu \
    sal \
))

$(eval $(call gb_Library_use_externals,vclplug_gtk,\
	boost_headers \
	cairo \
	dbus \
	gio \
	gtk \
	gthread \
	icuuc \
))

$(eval $(call gb_Library_add_libs,vclplug_gtk,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_gtk,\
    vcl/unx/gtk/a11y/atkaction \
    vcl/unx/gtk/a11y/atkbridge \
    vcl/unx/gtk/a11y/atkcomponent \
    vcl/unx/gtk/a11y/atkeditabletext \
    vcl/unx/gtk/a11y/atkfactory \
    vcl/unx/gtk/a11y/atkhypertext \
    vcl/unx/gtk/a11y/atkimage \
    vcl/unx/gtk/a11y/atklistener \
    vcl/unx/gtk/a11y/atkregistry \
    vcl/unx/gtk/a11y/atkselection \
    vcl/unx/gtk/a11y/atktable \
    vcl/unx/gtk/a11y/atktextattributes \
    vcl/unx/gtk/a11y/atktext \
    vcl/unx/gtk/a11y/atkutil \
    vcl/unx/gtk/a11y/atkvalue \
    vcl/unx/gtk/a11y/atkwindow \
    vcl/unx/gtk/a11y/atkwrapper \
    vcl/unx/gtk/app/gtkdata \
    vcl/unx/gtk/app/gtkinst \
    vcl/unx/gtk/app/gtksys \
    vcl/unx/gtk/gdi/salnativewidgets-gtk \
    vcl/unx/gtk/window/gtksalframe \
    vcl/unx/gtk/window/gtkobject \
    vcl/unx/gtk/fpicker/resourceprovider \
    vcl/unx/gtk/fpicker/SalGtkPicker \
    vcl/unx/gtk/fpicker/SalGtkFilePicker \
    vcl/unx/gtk/fpicker/SalGtkFolderPicker \
))

ifneq ($(ENABLE_DBUS),)
ifneq ($(ENABLE_GIO),)
$(eval $(call gb_Library_add_exception_objects,vclplug_gtk,\
    vcl/unx/gtk/window/gloactiongroup \
    vcl/unx/gtk/window/gtksalmenu \
    vcl/unx/gtk/window/glomenu \
    vcl/unx/gtk/window/hudawareness \
))
endif
endif

ifeq ($(ENABLE_GTK_PRINT),TRUE)
$(eval $(call gb_Library_add_exception_objects,vclplug_gtk,\
    vcl/unx/gtk/gdi/gtkprintwrapper \
    vcl/unx/gtk/gdi/salprn-gtk \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gtk,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
