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

$(eval $(call gb_Library_set_plugin_for,vclplug_gtk3,vcl))

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
    -DVCL_INTERNALS \
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
    vcl/unx/gtk3/a11y/atkaction \
    vcl/unx/gtk3/a11y/atkcomponent \
    vcl/unx/gtk3/a11y/atkeditabletext \
    vcl/unx/gtk3/a11y/atkfactory \
    vcl/unx/gtk3/a11y/atkhypertext \
    vcl/unx/gtk3/a11y/atkimage \
    vcl/unx/gtk3/a11y/atklistener \
    vcl/unx/gtk3/a11y/atkregistry \
    vcl/unx/gtk3/a11y/atkselection \
    vcl/unx/gtk3/a11y/atktable \
    vcl/unx/gtk3/a11y/atktablecell \
    vcl/unx/gtk3/a11y/atktextattributes \
    vcl/unx/gtk3/a11y/atktext \
    vcl/unx/gtk3/a11y/atkutil \
    vcl/unx/gtk3/a11y/atkvalue \
    vcl/unx/gtk3/a11y/atkwrapper \
    vcl/unx/gtk3/fpicker/resourceprovider \
    vcl/unx/gtk3/fpicker/SalGtkFilePicker \
    vcl/unx/gtk3/fpicker/SalGtkFolderPicker \
    vcl/unx/gtk3/fpicker/SalGtkPicker \
    vcl/unx/gtk3/customcellrenderer \
    vcl/unx/gtk3/gtkdata \
    vcl/unx/gtk3/gtkinst \
    vcl/unx/gtk3/gtksys \
    vcl/unx/gtk3/gtkcairo \
    vcl/unx/gtk3/salnativewidgets-gtk \
    vcl/unx/gtk3/gtkframe \
    vcl/unx/gtk3/gtkobject \
	vcl/unx/gtk3/gtksalmenu \
	vcl/unx/gtk3/glomenu \
	vcl/unx/gtk3/gloactiongroup \
    vcl/unx/gtk3/hudawareness \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gtk3,\
	-lm \
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
