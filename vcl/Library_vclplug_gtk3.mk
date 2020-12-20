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

$(eval $(call gb_Library_Library,vclplug_gtk3))

# Silence deprecation warnings wholesale as long as vcl/unx/gtk3/*.cxx just
# forward to vcl/unx/gtk/*.cxx:
$(eval $(call gb_Library_add_cxxflags,vclplug_gtk3, \
    -Wno-deprecated-declarations \
))

$(eval $(call gb_Library_set_include,vclplug_gtk3,\
    $$(INCLUDE) \
    $$(GTK3_CFLAGS) \
    $$(GSTREAMER_1_0_CFLAGS) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/unx \
    -I$(SRCDIR)/vcl/unx/gtk3 \
))

$(eval $(call gb_Library_add_defs,vclplug_gtk3,\
    -DVCLPLUG_GTK_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_custom_headers,vclplug_gtk3,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gtk3))

$(eval $(call gb_Library_add_libs,vclplug_gtk3,\
	$(GTK3_LIBS) \
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_use_libraries,vclplug_gtk3,\
    vcl \
    svl \
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

$(eval $(call gb_Library_use_externals,vclplug_gtk3,\
	boost_headers \
	epoxy \
	dbus \
	graphite \
	harfbuzz \
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
    vcl/unx/gtk3/a11y/gtk3atkwrapper \
    vcl/unx/gtk3/fpicker/resourceprovider \
    vcl/unx/gtk3/fpicker/SalGtkFilePicker \
    vcl/unx/gtk3/fpicker/SalGtkFolderPicker \
    vcl/unx/gtk3/fpicker/SalGtkPicker \
    vcl/unx/gtk3/gtk3gtkdata \
    vcl/unx/gtk3/gtk3gtkinst \
    vcl/unx/gtk3/gtk3gtksys \
    vcl/unx/gtk3/cairo_gtk3_cairo \
    vcl/unx/gtk3/gtk3gtkprintwrapper \
    vcl/unx/gtk3/gtk3salnativewidgets-gtk \
    vcl/unx/gtk3/gtk3gtkframe \
    vcl/unx/gtk3/gtk3gtkobject \
	vcl/unx/gtk3/gtk3gtksalmenu \
	vcl/unx/gtk3/gtk3glomenu \
	vcl/unx/gtk3/gtk3gloactiongroup \
    vcl/unx/gtk3/gtk3hudawareness \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gtk3,\
	-lm \
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
