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

$(eval $(call gb_Library_Library,vclplug_qt6))

$(eval $(call gb_Library_set_plugin_for,vclplug_qt6,vcl))

$(eval $(call gb_Library_use_custom_headers,vclplug_qt6,vcl/qt6))

$(eval $(call gb_Library_set_include,vclplug_qt6,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/vcl/inc/qt6 \
))

$(eval $(call gb_Library_add_defs,vclplug_qt6,\
    -DVCLPLUG_QT_IMPLEMENTATION \
    -DVCL_INTERNALS \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_qt6))

$(eval $(call gb_Library_use_libraries,vclplug_qt6,\
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

$(eval $(call gb_Library_use_externals,vclplug_qt6,\
    boost_headers \
    cairo \
    epoxy \
    graphite \
    harfbuzz \
    icu_headers \
    icuuc \
    qt6 \
))

ifneq ($(QT6_HAVE_GOBJECT),)
$(eval $(call gb_Library_add_cxxflags,vclplug_qt6,\
    $(QT6_GOBJECT_CFLAGS) \
))
$(eval $(call gb_Library_add_libs,vclplug_qt6,\
    $(QT6_GOBJECT_LIBS) \
))
endif

$(eval $(call gb_Library_add_exception_objects,vclplug_qt6,\
    vcl/qt6/QtAccessibleEventListener \
    vcl/qt6/QtAccessibleRegistry \
    vcl/qt6/QtAccessibleWidget \
    vcl/qt6/QtBitmap \
    vcl/qt6/QtBuilder \
    vcl/qt6/QtClipboard \
    vcl/qt6/QtCustomStyle \
    vcl/qt6/QtData \
    vcl/qt6/QtDoubleSpinBox \
    vcl/qt6/QtDragAndDrop \
    vcl/qt6/QtExpander \
    vcl/qt6/QtFilePicker \
    vcl/qt6/QtFont \
    vcl/qt6/QtFontFace \
    vcl/qt6/QtFrame \
    vcl/qt6/QtGraphics \
    vcl/qt6/QtGraphicsBase \
    vcl/qt6/QtGraphics_Controls \
    vcl/qt6/QtGraphics_GDI \
    vcl/qt6/QtGraphics_Text \
    vcl/qt6/QtHyperlinkLabel \
    vcl/qt6/QtInstance \
    vcl/qt6/QtInstance_Print \
    vcl/qt6/QtInstanceBox \
    vcl/qt6/QtInstanceBuilder \
    vcl/qt6/QtInstanceButton \
    vcl/qt6/QtInstanceCheckButton \
    vcl/qt6/QtInstanceComboBox \
    vcl/qt6/QtInstanceContainer \
    vcl/qt6/QtInstanceDialog \
    vcl/qt6/QtInstanceDrawingArea \
    vcl/qt6/QtInstanceEntry \
    vcl/qt6/QtInstanceExpander \
    vcl/qt6/QtInstanceFormattedSpinButton \
    vcl/qt6/QtInstanceFrame \
    vcl/qt6/QtInstanceGrid \
    vcl/qt6/QtInstanceIconView \
    vcl/qt6/QtInstanceImage \
    vcl/qt6/QtInstanceLabel \
    vcl/qt6/QtInstanceLevelBar \
    vcl/qt6/QtInstanceLinkButton \
    vcl/qt6/QtInstanceMenu \
    vcl/qt6/QtInstanceMenuButton \
    vcl/qt6/QtInstanceMessageDialog \
    vcl/qt6/QtInstanceNotebook \
    vcl/qt6/QtInstancePopover \
    vcl/qt6/QtInstanceProgressBar \
    vcl/qt6/QtInstanceRadioButton \
    vcl/qt6/QtInstanceScale \
    vcl/qt6/QtInstanceScrolledWindow \
    vcl/qt6/QtInstanceSpinButton \
    vcl/qt6/QtInstanceTextView \
    vcl/qt6/QtInstanceToggleButton \
    vcl/qt6/QtInstanceToolbar \
    vcl/qt6/QtInstanceTreeIter \
    vcl/qt6/QtInstanceTreeView \
    vcl/qt6/QtInstanceWidget \
    vcl/qt6/QtInstanceWindow \
    vcl/qt6/QtMainWindow \
    vcl/qt6/QtMenu \
    vcl/qt6/QtObject \
    vcl/qt6/QtOpenGLContext \
    vcl/qt6/QtPainter \
    vcl/qt6/QtPrinter \
    vcl/qt6/QtSvpGraphics \
    vcl/qt6/QtSvpSurface \
    vcl/qt6/QtSystem \
    vcl/qt6/QtTimer \
    vcl/qt6/QtTools \
    vcl/qt6/QtTransferable \
    vcl/qt6/QtVirtualDevice \
    vcl/qt6/QtWidget \
    vcl/qt6/QtXAccessible \
    $(if $(USING_X11), \
        vcl/qt6/QtX11Support \
    ) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclplug_qt6,\
	-lm \
	-ldl \
))
endif

# Workaround for clang+icecream (clang's -frewrite-includes
# doesn't handle Qt6's QT_HAS_INCLUDE that Qt6 uses for <chrono>).
ifeq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_Library_add_cxxflags,vclplug_qt6, \
    -include chrono \
))
endif

# vim: set noet sw=4 ts=4:
