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

$(eval $(call gb_Library_Library,vclplug_osx))

$(eval $(call gb_Library_set_plugin_for,vclplug_osx,vcl))

$(eval $(call gb_Library_set_include,vclplug_osx,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Library_use_sdk_api,vclplug_osx))

$(eval $(call gb_Library_use_custom_headers,vclplug_osx,\
    officecfg/registry \
))

# TODO: arguably the private CoreUI framework should never be used, no matter whether building
# a sandboxed version or a "regular" desktop version
$(eval $(call gb_Library_add_libs,vclplug_osx,\
    -framework IOKit \
    $(if $(ENABLE_MACOSX_SANDBOX),,\
        -F/System/Library/PrivateFrameworks \
        -framework CoreUI \
    ) \
    -lobjc \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_osx,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_Library_use_libraries,vclplug_osx,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    sal \
    salhelper \
    tl \
))

$(eval $(call gb_Library_use_externals,vclplug_osx,\
    boost_headers \
    epoxy \
    harfbuzz \
    $(if $(filter SKIA,$(BUILD_TYPE)), \
        skia \
    ) \
))

$(eval $(call gb_Library_add_defs,vclplug_osx,\
    -DMACOSX_BUNDLE_IDENTIFIER=\"$(MACOSX_BUNDLE_IDENTIFIER)\" \
    -DVCL_INTERNALS \
))

$(eval $(call gb_Library_add_objcxxobjects,vclplug_osx,\
    vcl/osx/a11yactionwrapper \
    vcl/osx/a11ycomponentwrapper \
    vcl/osx/a11yfactory \
    vcl/osx/a11yrolehelper \
    vcl/osx/a11yselectionwrapper \
    vcl/osx/a11ytablewrapper \
    vcl/osx/a11ytextattributeswrapper \
    vcl/osx/a11ytextwrapper \
    vcl/osx/a11yutil \
    vcl/osx/a11yvaluewrapper \
    vcl/osx/a11ywrapper \
    vcl/osx/a11ywrapperbutton \
    vcl/osx/a11ywrappercheckbox \
    vcl/osx/a11ywrappercombobox \
    vcl/osx/a11ywrappergroup \
    vcl/osx/a11ywrapperlist \
    vcl/osx/a11ywrapperradiobutton \
    vcl/osx/a11ywrapperradiogroup \
    vcl/osx/a11ywrapperrow \
    vcl/osx/a11ywrapperscrollarea \
    vcl/osx/a11ywrapperscrollbar \
    vcl/osx/a11ywrappersplitter \
    vcl/osx/a11ywrapperstatictext \
    vcl/osx/a11ywrappertabgroup \
    vcl/osx/a11ywrappertextarea \
    vcl/osx/a11ywrappertoolbar \
    vcl/osx/printaccessoryview \
    vcl/osx/printview \
    vcl/osx/salframeview \
    vcl/osx/salnsmenu \
    vcl/osx/salnstimer \
    vcl/osx/vclnsapp \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_osx,\
    vcl/osx/DataFlavorMapping \
    vcl/osx/DragActionConversion \
    vcl/osx/DragSource \
    vcl/osx/DragSourceContext \
    vcl/osx/DropTarget \
    vcl/osx/HtmlFmtFlt \
    vcl/osx/OSXTransferable \
    vcl/osx/PictToBmpFlt \
    vcl/osx/a11yfocuslistener \
    vcl/osx/a11yfocustracker \
    vcl/osx/a11ylistener \
    vcl/osx/clipboard \
    vcl/osx/documentfocuslistener \
    vcl/osx/saldata \
    vcl/osx/salframe \
    vcl/osx/salgdiutils \
    vcl/osx/salinst \
    vcl/osx/salmacos \
    vcl/osx/salmenu \
    vcl/osx/salnativewidgets \
    vcl/osx/salobj \
    vcl/osx/salprn \
    vcl/osx/salsys \
    vcl/osx/saltimer \
    vcl/osx/service_entry \
    vcl/quartz/CoreTextFont \
    vcl/quartz/CoreTextFontFace \
    vcl/quartz/SystemFontList \
    vcl/quartz/salbmp \
    vcl/quartz/salgdi \
    vcl/quartz/salgdicommon \
    vcl/quartz/salvd \
    vcl/quartz/utils \
    vcl/quartz/AquaGraphicsBackend \
    $(if $(filter SKIA,$(BUILD_TYPE)), \
        vcl/skia/osx/bitmap \
        vcl/skia/osx/gdiimpl \
        ) \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,vclplug_osx,\
    ApplicationServices \
    Cocoa \
    Carbon \
    CoreFoundation \
))

ifneq ($(ENABLE_MACOSX_SANDBOX),TRUE)
$(eval $(call gb_Library_use_libraries,vclplug_osx,\
    AppleRemote \
))
endif

# vim: set noet sw=4 ts=4:
