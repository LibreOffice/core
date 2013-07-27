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



$(eval $(call gb_Library_Library,vclplug_gen))

$(eval $(call gb_Library_set_include,vclplug_gen,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/pch \
    -I$(SRCDIR)/solenv/inc \
    -I$(OUTDIR)/inc/offuh \
    -I$(OUTDIR)/inc/stl \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_Library_add_linked_libs,vclplug_gen,\
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

$(eval $(call gb_Library_add_exception_objects,vclplug_gen,\
    vcl/unx/generic/app/i18n_cb \
    vcl/unx/generic/app/i18n_ic \
    vcl/unx/generic/app/i18n_im \
    vcl/unx/generic/app/i18n_keysym \
    vcl/unx/generic/app/i18n_status \
    vcl/unx/generic/app/i18n_wrp \
    vcl/unx/generic/app/i18n_xkb \
    vcl/unx/generic/app/keysymnames \
    vcl/unx/generic/app/randrwrapper \
    vcl/unx/generic/app/saldata \
    vcl/unx/generic/app/saldisp \
    vcl/unx/generic/app/salinst \
    vcl/unx/generic/app/salsys \
    vcl/unx/generic/app/saltimer \
    vcl/unx/generic/app/sm \
    vcl/unx/generic/app/soicon \
    vcl/unx/generic/app/wmadaptor \
    vcl/unx/generic/dtrans/bmp \
    vcl/unx/generic/dtrans/config \
    vcl/unx/generic/dtrans/X11_clipboard \
    vcl/unx/generic/dtrans/X11_dndcontext \
    vcl/unx/generic/dtrans/X11_droptarget \
    vcl/unx/generic/dtrans/X11_selection \
    vcl/unx/generic/dtrans/X11_service \
    vcl/unx/generic/dtrans/X11_transferable \
    vcl/unx/generic/gdi/cdeint \
    vcl/unx/generic/gdi/dtint \
    vcl/unx/generic/gdi/gcach_xpeer \
    vcl/unx/generic/gdi/pspgraphics \
    vcl/unx/generic/gdi/salbmp \
    vcl/unx/generic/gdi/salcvt \
    vcl/unx/generic/gdi/salgdi2 \
    vcl/unx/generic/gdi/salgdi3 \
    vcl/unx/generic/gdi/salgdi \
    vcl/unx/generic/gdi/salprnpsp \
    vcl/unx/generic/gdi/salvd \
    vcl/unx/generic/gdi/xrender_peer \
    vcl/unx/generic/printergfx/bitmap_gfx \
    vcl/unx/generic/printergfx/common_gfx \
    vcl/unx/generic/printergfx/glyphset \
    vcl/unx/generic/printergfx/printerjob \
    vcl/unx/generic/printergfx/psputil \
    vcl/unx/generic/printergfx/text_gfx \
    vcl/unx/generic/window/FWS \
    vcl/unx/generic/window/salframe \
    vcl/unx/generic/window/salobj \
))

$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -D_XSALSET_LIBNAME=\"$(call gb_Library_get_runtime_filename,spa)\" \
    -DVCLPLUG_GEN_IMPLEMENTATION \
))


## handle RandR 
ifneq ($(ENABLE_RANDR),)
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -DUSE_RANDR \
))
ifeq ($(XRANDR_DLOPEN),FALSE)
$(eval $(call gb_Library_set_cxxflags,vclplug_gen,\
    $$(CXXFLAGS) \
))
$(eval $(call gb_Library_set_ldflags,vclplug_gen,\
    $$(LDFLAGS) \
    $(XRANDR_LIBS) \
))
else
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -DXRANDR_DLOPEN \
))
endif
endif

## handle Xinerama
ifneq ($(USE_XINERAMA),NO)
ifneq ($(OS),SOLARIS)
# not Solaris
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -DUSE_XINERAMA \
    -DUSE_XINERAMA_XORG \
))
ifeq ($(XINERAMA_LINK),dynamic)
$(eval $(call gb_Library_set_ldflags,vclplug_gen,\
    $$(LDFLAGS) \
))

else
$(eval $(call gb_Library_set_ldflags,vclplug_gen,\
    $$(LDFLAGS) \
    -Wl,-Bstatic -lXinerama -Wl,-Bdynamic \
))
endif
else
# Solaris
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -DUSE_XINERAMA \
    -DUSE_XINERAMA_XSUN \
))
ifeq ($(USE_XINERAMA_VERSION),Xorg)
# Solaris, Xorg
ifeq ($(XINERAMA_LINK),dynamic)
$(eval $(call gb_Library_set_ldflags,vclplug_gen,\
    $$(LDFLAGS) \
    -lXinerama \
))
else
$(eval $(call gb_Library_set_ldflags,vclplug_gen,\
    $$(LDFLAGS) \
    -Wl,-Bstatic -lXinerama -Wl,-Bdynamic \
))
endif
endif
endif
endif

## handle Render linking
ifeq ($(XRENDER_LINK),YES)
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
    -DXRENDER_LINK \
))
$(eval $(call gb_Library_set_ldflags,vclplug_gen,\
    $$(LDFLAGS) \
    $(shell pkg-config --libs xrender) \
))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,vclplug_gen,\
    dl \
    m \
    pthread \
))
$(eval $(call gb_Library_add_external_libs,vclplug_gen,	\
    Xinerama Xrandr					\
))
endif
ifeq ($(OS),FREEBSD)
$(eval $(call gb_Library_add_external_libs,vclplug_gen,	\
    Xinerama Xrandr					\
))
endif
# vim: set noet sw=4 ts=4:
