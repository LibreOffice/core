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

$(eval $(call gb_Library_Library,vclplug_gen))

$(eval $(call gb_Library_set_plugin_for,vclplug_gen,vcl))

$(eval $(call gb_Library_set_include,vclplug_gen,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_use_custom_headers,vclplug_gen,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gen))

$(eval $(call gb_Library_use_common_precompiled_header,vclplug_gen))

$(eval $(call gb_Library_use_libraries,vclplug_gen,\
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

$(eval $(call gb_Library_use_externals,vclplug_gen,\
	boost_headers \
	cairo \
	graphite \
	epoxy \
	expat \
	harfbuzz \
	icu_headers \
	icuuc \
	valgrind \
	Xrender \
	$(if $(filter SKIA,$(BUILD_TYPE)), \
	    skia \
	    fontconfig \
	) \
))

$(eval $(call gb_Library_add_libs,vclplug_gen,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_gen,\
    vcl/unx/generic/app/i18n_cb \
    vcl/unx/generic/app/i18n_ic \
    vcl/unx/generic/app/i18n_im \
    vcl/unx/generic/app/i18n_keysym \
    vcl/unx/generic/app/i18n_xkb \
    vcl/unx/generic/app/keysymnames \
    vcl/unx/generic/app/randrwrapper \
    vcl/unx/generic/app/saldata \
    vcl/unx/generic/app/saldisp \
    vcl/unx/generic/app/salinst \
    vcl/unx/generic/app/saltimer \
    vcl/unx/generic/app/sm \
    vcl/unx/generic/app/wmadaptor \
    vcl/unx/generic/dtrans/bmp \
    vcl/unx/generic/dtrans/config \
    vcl/unx/generic/dtrans/X11_clipboard \
    vcl/unx/generic/dtrans/X11_dndcontext \
    vcl/unx/generic/dtrans/X11_droptarget \
    vcl/unx/generic/dtrans/X11_selection \
    vcl/unx/generic/dtrans/X11_service \
    vcl/unx/generic/dtrans/X11_transferable \
    vcl/unx/generic/gdi/cairo_xlib_cairo \
    vcl/unx/generic/gdi/X11CairoSalGraphicsImpl \
    vcl/unx/generic/gdi/font \
    vcl/unx/generic/gdi/salgdi \
    vcl/unx/generic/gdi/salvd \
    vcl/unx/generic/window/salframe \
    vcl/unx/generic/window/salobj \
    vcl/unx/x11/x11sys \
    vcl/unx/x11/xlimits \
    vcl/source/opengl/x11/context \
    $(if $(filter SKIA,$(BUILD_TYPE)), \
        vcl/skia/x11/gdiimpl \
        vcl/skia/x11/salvd \
        vcl/skia/x11/textrender \
        ) \
))

# ultimately we want to split the x11 dependencies out
# into their own library I think.

$(eval $(call gb_Library_add_defs,vclplug_gen,\
    -DVCLPLUG_GEN_IMPLEMENTATION \
    -DVCL_INTERNALS \
))

## handle RandR
ifneq ($(ENABLE_RANDR),)
$(eval $(call gb_Library_use_externals,vclplug_gen,\
	Xrandr \
))
$(eval $(call gb_Library_add_defs,vclplug_gen,\
    -DUSE_RANDR \
))
endif

## handle Xinerama
ifneq ($(USING_X11),)
ifeq ($(XINERAMA_LINK),dynamic)
$(eval $(call gb_Library_add_libs,vclplug_gen,\
    -lXinerama \
))
else
$(eval $(call gb_Library_add_libs,vclplug_gen,\
    -Wl$(COMMA)-Bstatic -lXinerama -Wl$(COMMA)-Bdynamic \
))
endif
endif # USING_X11

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gen,\
	-lm \
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
