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

$(eval $(call gb_Library_set_plugin_for,vclplug_qt5,vcl))

$(eval $(call gb_Library_use_custom_headers,vclplug_qt5,vcl/qt5))

$(eval $(call gb_Library_set_include,vclplug_qt5,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/qt5 \
    $(GSTREAMER_1_0_CFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_qt5,\
    -DVCLPLUG_QT_IMPLEMENTATION \
    -DVCL_INTERNALS \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_qt5))

$(eval $(call gb_Library_use_libraries,vclplug_qt5,\
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
    xmlreader \
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

ifneq ($(QT5_HAVE_GOBJECT),)
$(eval $(call gb_Library_add_cxxflags,vclplug_qt5,\
    $(QT5_GOBJECT_CFLAGS) \
))
$(eval $(call gb_Library_add_libs,vclplug_qt5,\
    $(QT5_GOBJECT_LIBS) \
))
endif

$(eval $(call gb_Library_add_exception_objects,vclplug_qt5,\
    vcl/qt5/QtAccessibleEventListener \
    vcl/qt5/QtAccessibleRegistry \
    vcl/qt5/QtAccessibleWidget \
    vcl/qt5/QtBitmap \
    vcl/qt5/QtBuilder \
    vcl/qt5/QtClipboard \
    vcl/qt5/QtCustomStyle \
    vcl/qt5/QtData \
    vcl/qt5/QtDoubleSpinBox \
    vcl/qt5/QtDragAndDrop \
    vcl/qt5/QtExpander \
    vcl/qt5/QtFilePicker \
    vcl/qt5/QtFont \
    vcl/qt5/QtFontFace \
    vcl/qt5/QtFrame \
    vcl/qt5/QtGraphics \
    vcl/qt5/QtGraphicsBase \
    vcl/qt5/QtGraphics_Controls \
    vcl/qt5/QtGraphics_GDI \
    vcl/qt5/QtGraphics_Text \
    vcl/qt5/QtHyperlinkLabel \
    vcl/qt5/QtInstance \
    vcl/qt5/QtInstance_Print \
    vcl/qt5/QtInstanceBuilder \
    vcl/qt5/QtInstanceButton \
    vcl/qt5/QtInstanceCheckButton \
    vcl/qt5/QtInstanceContainer \
    vcl/qt5/QtInstanceComboBox \
    vcl/qt5/QtInstanceDialog \
    vcl/qt5/QtInstanceDrawingArea \
    vcl/qt5/QtInstanceEntry \
    vcl/qt5/QtInstanceExpander \
    vcl/qt5/QtInstanceFrame \
    vcl/qt5/QtInstanceGrid \
    vcl/qt5/QtInstanceIconView \
    vcl/qt5/QtInstanceImage \
    vcl/qt5/QtInstanceLabel \
    vcl/qt5/QtInstanceLevelBar \
    vcl/qt5/QtInstanceLinkButton \
    vcl/qt5/QtInstanceMenu \
    vcl/qt5/QtInstanceMenuButton \
    vcl/qt5/QtInstanceMessageDialog \
    vcl/qt5/QtInstanceNotebook \
    vcl/qt5/QtInstancePopover \
    vcl/qt5/QtInstanceProgressBar \
    vcl/qt5/QtInstanceRadioButton \
    vcl/qt5/QtInstanceScale \
    vcl/qt5/QtInstanceScrolledWindow \
    vcl/qt5/QtInstanceSpinButton \
    vcl/qt5/QtInstanceTextView \
    vcl/qt5/QtInstanceToggleButton \
    vcl/qt5/QtInstanceToolbar \
    vcl/qt5/QtInstanceTreeView \
    vcl/qt5/QtInstanceWidget \
    vcl/qt5/QtInstanceWindow \
    vcl/qt5/QtMainWindow \
    vcl/qt5/QtMenu \
    vcl/qt5/QtObject \
    vcl/qt5/QtOpenGLContext \
    vcl/qt5/QtPainter \
    vcl/qt5/QtPrinter \
    vcl/qt5/QtSvpGraphics \
    vcl/qt5/QtSvpSurface \
    vcl/qt5/QtSystem \
    vcl/qt5/QtTimer \
    vcl/qt5/QtTools \
    vcl/qt5/QtTransferable \
    vcl/qt5/QtVirtualDevice \
    vcl/qt5/QtWidget \
    vcl/qt5/QtXAccessible \
    $(if $(USING_X11), \
        vcl/qt5/QtX11Support \
    ) \
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
