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

$(eval $(call gb_Library_Library,vclplug_kde4))

$(eval $(call gb_Library_use_custom_headers,vclplug_kde4,vcl/unx/kde4))

$(eval $(call gb_Library_set_include,vclplug_kde4,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,vclplug_kde4,\
    -DVCLPLUG_KDE4_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_kde4))

$(eval $(call gb_Library_use_libraries,vclplug_kde4,\
    vclplug_gen \
    vcl \
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

$(eval $(call gb_Library_use_externals,vclplug_kde4,\
	boost_headers \
	icuuc \
	kde4 \
))

$(eval $(call gb_Library_add_libs,vclplug_kde4,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

ifneq ($(KDE_HAVE_GLIB),)
$(eval $(call gb_Library_add_defs,vclplug_kde4,\
    $(KDE_GLIB_CFLAGS) \
))

$(eval $(call gb_Library_add_libs,vclplug_kde4,\
    $(KDE_GLIB_LIBS) \
))
endif


$(eval $(call gb_Library_add_exception_objects,vclplug_kde4,\
    vcl/unx/kde4/KDEData \
	vcl/unx/kde4/KDE4FilePicker \
    vcl/unx/kde4/KDESalDisplay \
    vcl/unx/kde4/KDESalFrame \
    vcl/unx/kde4/KDESalGraphics \
    vcl/unx/kde4/KDESalInstance \
    vcl/unx/kde4/KDEXLib \
    vcl/unx/kde4/main \
    vcl/unx/kde4/VCLKDEApplication \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_kde4,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
