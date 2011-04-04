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
    $$(XRANDR_CFLAGS) \
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
    -DUSE_XINERAMA_XORG \
))
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
else
# Solaris
$(eval $(call gb_Library_set_defs,vclplug_gen,\
    $$(DEFS) \
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
endif
# vim: set noet sw=4 ts=4:
