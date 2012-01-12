#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,vclplug_gtk))

$(eval $(call gb_Library_set_include,vclplug_gtk,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_cxxflags,vclplug_gtk,\
    $$(CXXFLAGS) \
    $$(GTK_CFLAGS) \
))

$(eval $(call gb_Library_set_defs,vclplug_gtk,\
    $$(DEFS) \
    -DVCLPLUG_GTK_IMPLEMENTATION \
    -DVERSION=\"$(UPD)$(LAST_MINOR)\" \
))

ifneq ($(ENABLE_DBUS),)
$(eval $(call gb_Library_set_include,vclplug_gtk,\
    $$(INCLUDE) \
    $(shell pkg-config --cflags-only-I dbus-glib-1) \
))
$(eval $(call gb_Library_set_defs,vclplug_gtk,\
    $$(DEFS) \
    -DENABLE_DBUS \
))
$(eval $(call gb_Library_set_ldflags,vclplug_gtk,\
    $$(LDFLAGS) \
    $(filter-out -l%,$(shell pkg-config --libs dbus-glib-1)) \
))
$(eval $(call gb_Library_add_external_libs,vclplug_gtk, \
    $(patsubst -l%,%, $(filter -l%, $(shell pkg-config --libs dbus-glib-1))) \
))
endif

$(eval $(call gb_Library_set_ldflags,vclplug_gtk,	\
    $$(LDFLAGS)						\
    $(filter-out -l%,$(GTK_LIBS))			\
))
$(eval $(call gb_Library_add_external_libs,vclplug_gtk,	\
    $(patsubst -l%,%, $(filter -l%, $(GTK_LIBS)))	\
))

$(eval $(call gb_Library_add_linked_libs,vclplug_gtk,\
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
    $(gb_STDLIBS) \
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
    vcl/unx/gtk/window/gtkframe \
    vcl/unx/gtk/window/gtkobject \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vclplug_gtk,\
    dl \
    m \
    pthread \
))
endif

# vim: set noet sw=4 ts=4:
