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

$(eval $(call gb_Library_set_componentfile,vclplug_osx,vcl/vcl.common))

$(eval $(call gb_Library_set_include,vclplug_osx,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))


ifeq ($(SYSTEM_GLM),TRUE)
$(eval $(call gb_Library_add_defs,vclplug_osx,\
    -DGLM_ENABLE_EXPERIMENTAL \
))
endif

$(eval $(call gb_Library_use_sdk_api,vclplug_osx))

$(eval $(call gb_Library_use_custom_headers,vclplug_osx,\
    officecfg/registry \
))

$(eval $(call gb_Library_add_libs,vclplug_osx,\
    -framework IOKit \
    -F/System/Library/PrivateFrameworks \
    -framework CoreUI \
    -lobjc \
))

$(eval $(call gb_Library_add_cxxflags,vclplug_osx,\
    $(gb_OBJCXXFLAGS) \
))

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Library_use_libraries,vclplug_osx,\
    jvmaccess \
))
endif

$(eval $(call gb_Library_use_libraries,vclplug_osx,\
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

$(eval $(call gb_Library_use_externals,vclplug_osx,\
    boost_headers \
    gio \
    glm_headers \
    graphite \
    harfbuzz \
    icu_headers \
    icuuc \
    lcms2 \
    mdds_headers \
))
ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Library_use_externals,vclplug_osx,\
     epoxy \
 ))
endif

# optional parts

vcl_quartz_code= \
    vcl/quartz/salbmp \
    vcl/quartz/utils \
    vcl/quartz/salgdicommon \
    vcl/quartz/salvd \

vcl_coretext_code= \
    vcl/quartz/ctfonts \
    vcl/quartz/salgdi \

$(eval $(call gb_Library_add_cxxflags,vclplug_osx,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_Library_add_defs,vclplug_osx,\
    -DMACOSX_BUNDLE_IDENTIFIER=\"$(MACOSX_BUNDLE_IDENTIFIER)\" \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_osx,\
    $(vcl_coretext_code) \
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
    vcl/osx/salnstimer \
    vcl/osx/vclnsapp \
    vcl/osx/printaccessoryview \
    vcl/osx/printview \
    vcl/osx/salframeview \
    vcl/osx/salnsmenu \
))

$(eval $(call gb_Library_add_exception_objects,vclplug_osx,\
    vcl/osx/a11yfocuslistener \
    vcl/osx/a11yfocustracker \
    vcl/osx/a11ylistener \
    vcl/osx/documentfocuslistener \
    vcl/osx/saldata \
    vcl/osx/salinst \
    vcl/osx/salsys \
    vcl/osx/saltimer \
    vcl/osx/DataFlavorMapping \
    vcl/osx/DragActionConversion \
    vcl/osx/DragSource \
    vcl/osx/DragSourceContext \
    vcl/osx/DropTarget \
    vcl/osx/HtmlFmtFlt \
    vcl/osx/OSXTransferable \
    vcl/osx/PictToBmpFlt \
    vcl/osx/clipboard \
    vcl/osx/service_entry \
    $(vcl_quartz_code) \
    vcl/quartz/salgdiutils \
    vcl/osx/salnativewidgets \
    vcl/osx/salprn \
    vcl/osx/salframe \
    vcl/osx/salmenu \
    vcl/osx/salobj \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,vclplug_osx,\
    ApplicationServices \
    Cocoa \
    Carbon \
    CoreFoundation \
    $(if $(filter X86_64,$(CPUNAME)),,QuickTime) \
))

ifneq ($(ENABLE_MACOSX_SANDBOX),TRUE)
$(eval $(call gb_Library_use_libraries,vclplug_osx,\
    AppleRemote \
))
endif

# vim: set noet sw=4 ts=4:
