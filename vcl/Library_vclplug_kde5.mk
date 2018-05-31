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

$(eval $(call gb_Library_Library,vclplug_kde5))

$(eval $(call gb_Library_use_custom_headers,vclplug_kde5,vcl/unx/kde5))

$(eval $(call gb_Library_set_include,vclplug_kde5,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_add_defs,vclplug_kde5,\
    -DVCLPLUG_KDE5_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_kde5))

$(eval $(call gb_Library_use_libraries,vclplug_kde5,\
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

$(eval $(call gb_Library_use_externals,vclplug_kde5,\
	boost_headers \
	cairo \
	graphite \
	harfbuzz \
	icuuc \
	kde5 \
	epoxy \
))

$(eval $(call gb_Library_add_libs,vclplug_kde5,\
	-lX11 \
	-lXext \
	-lSM \
	-lICE \
))

ifneq ($(QT5_HAVE_GLIB),)
$(eval $(call gb_Library_add_defs,vclplug_kde5,\
    $(QT5_GLIB_CFLAGS) \
))

$(eval $(call gb_Library_add_libs,vclplug_kde5,\
    $(QT5_GLIB_LIBS) \
))
endif

$(eval $(call gb_Library_add_defs,vclplug_kde5,\
    $(KF5_CFLAGS) \
))
$(eval $(call gb_Library_add_libs,vclplug_kde5,\
    $(KF5_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_kde5,\
    vcl/unx/kde5/KDE5Data \
    vcl/unx/kde5/KDE5SalDisplay \
    vcl/unx/kde5/KDE5SalFrame \
    vcl/unx/kde5/KDE5SalGraphics \
    vcl/unx/kde5/KDE5SalInstance \
    vcl/unx/kde5/KDE5XLib \
    vcl/unx/kde5/VCLKDE5Application \
    vcl/unx/kde5/main \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_kde5,\
	-lm \
	-ldl \
	-lpthread \
))
endif

# vim: set noet sw=4 ts=4:
