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

$(eval $(call gb_Library_Library,vclplug_qt5))

$(eval $(call gb_Library_use_custom_headers,vclplug_qt5,vcl/qt5))

$(eval $(call gb_Library_set_include,vclplug_qt5,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/qt5 \
    $(GSTREAMER_1_0_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_qt5,\
    -DVCLPLUG_QT5_IMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_qt5))

$(eval $(call gb_Library_use_libraries,vclplug_qt5,\
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
    salhelper \
))

$(eval $(call gb_Library_use_externals,vclplug_qt5,\
    boost_headers \
    cairo \
    epoxy \
    graphite \
    harfbuzz \
    icu_headers \
    icuuc \
    qt5 \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_qt5,\
    $(QT5_CFLAGS) \
))
$(eval $(call gb_Library_add_libs,vclplug_qt5,\
    $(QT5_LIBS) \
))

ifneq ($(QT5_HAVE_GOBJECT),)
$(eval $(call gb_Library_add_cxxflags,vclplug_qt5,\
    $(QT5_GOBJECT_CFLAGS) \
))
$(eval $(call gb_Library_add_libs,vclplug_qt5,\
    $(QT5_GOBJECT_LIBS) \
))
endif

$(eval $(call gb_Library_add_exception_objects,vclplug_qt5,\
    vcl/qt5/Qt5AccessibleEventListener \
    vcl/qt5/Qt5AccessibleWidget \
    vcl/qt5/Qt5Bitmap \
    vcl/qt5/Qt5Clipboard \
    vcl/qt5/Qt5Data \
    vcl/qt5/Qt5DragAndDrop \
    vcl/qt5/Qt5FilePicker \
    vcl/qt5/Qt5Font \
    vcl/qt5/Qt5FontFace \
    vcl/qt5/Qt5Frame \
    vcl/qt5/Qt5Graphics \
    vcl/qt5/Qt5Graphics_Controls \
    vcl/qt5/Qt5Graphics_GDI \
    vcl/qt5/Qt5Graphics_Text \
    vcl/qt5/Qt5Instance \
    vcl/qt5/Qt5Instance_Print \
    vcl/qt5/Qt5MainWindow \
    vcl/qt5/Qt5Menu \
    vcl/qt5/Qt5Object \
    vcl/qt5/Qt5OpenGLContext \
    vcl/qt5/Qt5Painter \
    vcl/qt5/Qt5Printer \
    vcl/qt5/Qt5SvpGraphics \
    vcl/qt5/Qt5SvpSurface \
    vcl/qt5/Qt5System \
    vcl/qt5/Qt5Timer \
    vcl/qt5/Qt5Tools \
    vcl/qt5/Qt5Transferable \
    vcl/qt5/Qt5VirtualDevice \
    vcl/qt5/Qt5Widget \
    vcl/qt5/Qt5XAccessible \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_qt5,\
	-lm \
	-ldl \
))
endif

# Workaround for clang+icecream (clang's -frewrite-includes
# doesn't handle Qt5's QT_HAS_INCLUDE that Qt5 uses for <chrono>).
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_Library_add_cxxflags,vclplug_qt5, \
    -include chrono \
))
endif

# vim: set noet sw=4 ts=4:
