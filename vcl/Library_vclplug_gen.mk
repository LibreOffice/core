# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
))

$(eval $(call gb_Library_use_sdk_api,vclplug_gen))

$(eval $(call gb_Library_use_libraries,vclplug_gen,\
    vcl \
    tl \
    utl \
    sot \
    ucbhelper \
    basegfx \
    comphelper \
    cppuhelper \
    i18nisolang1 \
    i18nutil \
    jvmaccess \
    cppu \
    sal \
))

$(eval $(call gb_Library_use_externals,vclplug_gen,\
	cairo \
	graphite \
	icule \
	icuuc \
	Xrender \
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
    vcl/unx/generic/app/i18n_status \
    vcl/unx/generic/app/i18n_xkb \
    vcl/unx/generic/app/keysymnames \
    vcl/unx/generic/app/randrwrapper \
    vcl/unx/generic/app/saldata \
    vcl/unx/generic/app/saldisp \
    vcl/unx/generic/app/salinst \
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
    vcl/unx/generic/gdi/gcach_xpeer \
    vcl/unx/generic/gdi/salbmp \
    vcl/unx/generic/gdi/salgdi2 \
    vcl/unx/generic/gdi/salgdi3 \
    vcl/unx/generic/gdi/salgdi \
    vcl/unx/generic/gdi/salvd \
    vcl/unx/generic/gdi/xrender_peer \
    vcl/unx/generic/window/FWS \
    vcl/unx/generic/window/salframe \
    vcl/unx/generic/window/salobj \
    vcl/unx/x11/x11sys \
    vcl/unx/x11/xlimits \
))

# ultimately we want to split the x11 dependencies out
# into their own library I think.

$(eval $(call gb_Library_add_defs,vclplug_gen,\
    -D_XSALSET_LIBNAME=\"$(call gb_Library_get_runtime_filename,spa)\" \
    -DVCLPLUG_GEN_IMPLEMENTATION \
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

$(eval $(call gb_Library_add_defs,vclplug_gen,\
    $(if $(VALGRIND_CFLAGS), \
        $(VALGRIND_CFLAGS) \
        -DHAVE_MEMCHECK_H=1 \
    ) \
))

## handle Xinerama
ifneq ($(USE_XINERAMA),NO)
ifneq ($(OS)$(USE_XINERAMA_VERSION),SOLARISXsun)
# not Solaris/Xsun
$(eval $(call gb_Library_add_defs,vclplug_gen,\
    -DUSE_XINERAMA_XORG \
))
ifeq ($(XINERAMA_LINK),dynamic)
$(eval $(call gb_Library_add_libs,vclplug_gen,\
    -lXinerama \
))
else
$(eval $(call gb_Library_add_libs,vclplug_gen,\
    -Wl,-Bstatic -lXinerama -Wl,-Bdynamic \
))
endif
else # Solaris/Xsun
$(eval $(call gb_Library_add_defs,vclplug_gen,\
    -DUSE_XINERAMA_XSUN \
))
endif
endif # USE_XINERAMA

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_gen,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
