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

$(eval $(call gb_Library_Library,vclplug_gtk4))

$(eval $(call gb_Library_set_plugin_for,vclplug_gtk4,vcl))

# Silence deprecation warnings wholesale as long as vcl/unx/gtk4/*.cxx just
# forward to vcl/unx/gtk/*.cxx:
$(eval $(call gb_Library_add_cxxflags,vclplug_gtk4, \
    -Wno-deprecated-declarations \
))

$(eval $(call gb_Library_set_include,vclplug_gtk4,\
    $$(INCLUDE) \
    $$(GTK4_CFLAGS) \
    $$(GSTREAMER_1_0_CFLAGS) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/unx \
    -I$(SRCDIR)/vcl/unx/gtk4 \
))

$(eval $(call gb_Library_add_defs,vclplug_gtk4,\
    -DVCLPLUG_GTK_IMPLEMENTATION \
    -DVCL_INTERNALS \
))

$(eval $(call gb_Library_use_custom_headers,vclplug_gtk4,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gtk4))

$(eval $(call gb_Library_add_libs,vclplug_gtk4,\
	$(GTK4_LIBS) \
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_use_libraries,vclplug_gtk4,\
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

$(eval $(call gb_Library_use_externals,vclplug_gtk4,\
	boost_headers \
	epoxy \
	dbus \
	graphite \
	harfbuzz \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_gtk4,\
    vcl/unx/gtk4/fpicker/resourceprovider \
    vcl/unx/gtk4/fpicker/SalGtkFilePicker \
    vcl/unx/gtk4/fpicker/SalGtkFolderPicker \
    vcl/unx/gtk4/fpicker/SalGtkPicker \
    vcl/unx/gtk4/a11y \
    vcl/unx/gtk4/convert3to4 \
    vcl/unx/gtk4/customcellrenderer \
    vcl/unx/gtk4/gtkaccessibleeventlistener \
    vcl/unx/gtk4/gtkaccessibleregistry \
    vcl/unx/gtk4/gtkaccessibletext \
    vcl/unx/gtk4/gtkdata \
    vcl/unx/gtk4/gtkinst \
    vcl/unx/gtk4/gtksys \
    vcl/unx/gtk4/gtkcairo \
    vcl/unx/gtk4/custom-theme \
    vcl/unx/gtk4/salnativewidgets-gtk \
    vcl/unx/gtk4/gtkframe \
    vcl/unx/gtk4/gtkobject \
    vcl/unx/gtk4/gtksalmenu \
    vcl/unx/gtk4/glomenu \
    vcl/unx/gtk4/gloactiongroup \
    vcl/unx/gtk4/hudawareness \
    vcl/unx/gtk4/notifyinglayout \
    vcl/unx/gtk4/surfacecellrenderer \
    vcl/unx/gtk4/surfacepaintable \
    vcl/unx/gtk4/transferableprovider \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gtk4,\
	-lm \
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
