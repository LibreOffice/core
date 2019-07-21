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

$(eval $(call gb_Library_Library,vclplug_kf5))

$(eval $(call gb_Library_use_custom_headers,vclplug_kf5,vcl/unx/kf5))

$(eval $(call gb_Library_set_include,vclplug_kf5,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/qt5 \
))

$(eval $(call gb_Library_add_defs,vclplug_kf5,\
    -DVCLPLUG_KF5_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_kf5))

$(eval $(call gb_Library_use_libraries,vclplug_kf5,\
    vclplug_qt5 \
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

$(eval $(call gb_Library_use_externals,vclplug_kf5,\
	boost_headers \
	cairo \
	graphite \
	harfbuzz \
	icuuc \
	kf5 \
	epoxy \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_kf5,\
    $(KF5_CFLAGS) \
))
$(eval $(call gb_Library_add_libs,vclplug_kf5,\
    $(KF5_LIBS) \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_kf5,\
    vcl/unx/kf5/KF5FilePicker \
    vcl/unx/kf5/KF5SalFrame \
    vcl/unx/kf5/KF5SalInstance \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_kf5,\
	-lm \
	-ldl \
))
endif

# Workaround for clang+icecream (clang's -frewrite-includes
# doesn't handle Qt5's QT_HAS_INCLUDE that Qt5 uses for <chrono>).
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_Library_add_cxxflags,vclplug_kf5, \
    -include chrono \
))
endif

# vim: set noet sw=4 ts=4:
