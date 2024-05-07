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

$(eval $(call gb_Library_Library,vclplug_gtk3_kde5))

$(eval $(call gb_Library_set_plugin_for,vclplug_gtk3_kde5,vcl))

# Silence deprecation warnings wholesale as long as vcl/unx/gtk3/*.cxx just
# forward to vcl/unx/gtk/*.cxx:
$(eval $(call gb_Library_add_cxxflags,vclplug_gtk3_kde5, \
    -Wno-deprecated-declarations \
))

$(eval $(call gb_Library_set_include,vclplug_gtk3_kde5,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/unx \
    -I$(SRCDIR)/vcl/unx/gtk3 \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_gtk3_kde5,\
    $$(INCLUDE) \
    $$(GTK3_CFLAGS) \
    $$(GSTREAMER_1_0_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_gtk3_kde5,\
    -DVCLPLUG_GTK_IMPLEMENTATION -DVCLPLUG_GTK3_KDE5_IMPLEMENTATION \
    -DVCL_INTERNALS \
))

$(eval $(call gb_Library_use_custom_headers,vclplug_gtk3_kde5,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gtk3_kde5))

$(eval $(call gb_Library_add_libs,vclplug_gtk3_kde5,\
	$(GTK3_LIBS) \
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_use_libraries,vclplug_gtk3_kde5,\
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

$(eval $(call gb_Library_use_externals,vclplug_gtk3_kde5,\
	boost_headers \
	boost_filesystem \
	epoxy \
	dbus \
	graphite \
	harfbuzz \
	kf5 \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_gtk3_kde5,\
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkaction \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkcomponent \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkeditabletext \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkfactory \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkhypertext \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkimage \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atklistener \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkregistry \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkselection \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atktable \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atktablecell \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atktextattributes \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atktext \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkutil \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkvalue \
	vcl/unx/gtk3_kde5/a11y/gtk3_kde5_atkwrapper \
	vcl/unx/gtk3_kde5/gtk3_kde5_customcellrenderer \
	vcl/unx/gtk3_kde5/gtk3_kde5_gtkdata \
	vcl/unx/gtk3_kde5/gtk3_kde5_gtkinst \
	vcl/unx/gtk3_kde5/gtk3_kde5_gtksys \
	vcl/unx/gtk3_kde5/gtk3_kde5_filepicker \
	vcl/unx/gtk3_kde5/gtk3_kde5_filepicker_ipc \
	vcl/unx/gtk3_kde5/gtk3_kde5_folderpicker \
	vcl/unx/gtk3_kde5/gtk3_kde5_cairo \
	vcl/unx/gtk3_kde5/gtk3_kde5_salnativewidgets-gtk \
	vcl/unx/gtk3_kde5/gtk3_kde5_gtkframe \
	vcl/unx/gtk3_kde5/gtk3_kde5_gtkobject \
	vcl/unx/gtk3_kde5/gtk3_kde5_gtksalmenu \
	vcl/unx/gtk3_kde5/gtk3_kde5_glomenu \
	vcl/unx/gtk3_kde5/gtk3_kde5_gloactiongroup \
	vcl/unx/gtk3_kde5/gtk3_kde5_hudawareness \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gtk3_kde5,\
	-lm \
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
