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



$(eval $(call gb_Library_Library,vclplug_kde4))

$(eval $(call gb_Library_add_api,vclplug_kde4,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_set_include,vclplug_kde4,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_set_include,vclplug_kde4,\
    $$(INCLUDE) \
    $$(KDE4_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_kde4,\
    -DVCLPLUG_KDE4_IMPLEMENTATION \
))

$(eval $(call gb_Library_add_libs,vclplug_kde4,\
    $$(KDE4_LIBS) \
))

$(eval $(call gb_Library_add_linked_libs,vclplug_kde4,\
    vclplug_gen \
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
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

$(call gb_Library_use_externals,vclplug_kde4,\
    icule \
    icuuc \
)

$(eval $(call gb_Library_add_exception_objects,vclplug_kde4,\
    vcl/unx/kde4/KDEData \
    vcl/unx/kde4/KDESalDisplay \
    vcl/unx/kde4/KDESalFrame \
    vcl/unx/kde4/KDESalGraphics \
    vcl/unx/kde4/KDESalInstance \
    vcl/unx/kde4/KDEXLib \
    vcl/unx/kde4/main \
    vcl/unx/kde4/VCLKDEApplication \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vclplug_kde4,\
    dl \
    m \
    pthread \
))
endif

# vim: set noet sw=4 ts=4:
