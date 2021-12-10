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

$(eval $(call gb_Library_Library,svt))

$(eval $(call gb_Library_set_precompiled_header,svt,svtools/inc/pch/precompiled_svt))

$(eval $(call gb_Library_set_componentfile,svt,svtools/util/svt))

$(eval $(call gb_Library_use_custom_headers,svt,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,svt))

$(eval $(call gb_Library_set_include,svt,\
    $$(INCLUDE) \
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc \
))

$(eval $(call gb_Library_add_defs,svt,\
    -DSVT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,svt,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    i18nutil \
    $(if $(ENABLE_JAVA), \
        jvmfwk) \
    salhelper \
    sal \
    sot \
    svl \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
))

$(eval $(call gb_Library_use_externals,svt,\
	boost_headers \
    icui18n \
    icuuc \
    icu_headers \
))

$(eval $(call gb_Library_add_exception_objects,svt,\
    svtools/source/brwbox/brwbox1 \
    svtools/source/brwbox/brwbox2 \
    svtools/source/brwbox/brwbox3 \
    svtools/source/brwbox/brwhead \
    svtools/source/brwbox/datwin \
    svtools/source/brwbox/ebbcontrols \
    svtools/source/brwbox/editbrowsebox \
    svtools/source/brwbox/editbrowsebox2 \
    svtools/source/brwbox/recorditemwindow \
    svtools/source/config/accessibilityoptions \
    svtools/source/config/apearcfg \
    svtools/source/config/colorcfg \
    svtools/source/config/extcolorcfg \
    svtools/source/config/fontsubstconfig \
    svtools/source/config/htmlcfg \
    svtools/source/config/itemholder2 \
    svtools/source/config/miscopt \
    svtools/source/config/slidesorterbaropt \
    svtools/source/config/optionsdrawinglayer \
    svtools/source/config/printoptions \
    svtools/source/contnr/querydelete \
    svtools/source/control/accessibleruler \
    svtools/source/control/asynclink \
    svtools/source/control/collatorres \
    svtools/source/control/ctrlbox \
    svtools/source/control/ctrltool \
    svtools/source/control/indexentryres \
    svtools/source/control/inettbc \
    svtools/source/control/ruler \
    svtools/source/control/scriptedtext \
    svtools/source/control/tabbar \
    svtools/source/control/toolbarmenu \
    svtools/source/control/valueacc \
    svtools/source/control/valueset \
    svtools/source/dialogs/addresstemplate \
    svtools/source/dialogs/colrdlg \
    svtools/source/dialogs/insdlg \
    svtools/source/dialogs/PlaceEditDialog \
    svtools/source/dialogs/prnsetup \
    svtools/source/dialogs/restartdialog \
    svtools/source/dialogs/ServerDetailsControls \
    svtools/source/filter/SvFilterOptionsDialog \
    svtools/source/filter/DocumentToGraphicRenderer \
    svtools/source/filter/exportdialog \
    svtools/source/graphic/renderer \
    $(if $(ENABLE_JAVA), \
        svtools/source/java/javacontext \
        svtools/source/java/javainteractionhandler) \
    svtools/source/misc/acceleratorexecute \
    svtools/source/misc/bindablecontrolhelper \
    svtools/source/misc/cliplistener \
    svtools/source/misc/dialogclosedlistener \
    svtools/source/misc/ehdl \
    svtools/source/misc/embedhlp \
    svtools/source/misc/embedtransfer \
    svtools/source/misc/filechangedchecker \
    svtools/source/misc/imagemgr \
    svtools/source/misc/imageresourceaccess \
    svtools/source/misc/langtab \
    svtools/source/misc/langhelp \
    svtools/source/misc/openfiledroptargetlistener \
    svtools/source/misc/sampletext \
    svtools/source/misc/stringtransfer \
    svtools/source/misc/svtresid \
    svtools/source/misc/templatefoldercache \
    svtools/source/misc/unitconv \
    svtools/source/svhtml/htmlkywd \
    svtools/source/svhtml/htmlout \
    svtools/source/svhtml/htmlsupp \
    svtools/source/svhtml/HtmlWriter \
    svtools/source/svhtml/parhtml \
    svtools/source/svrtf/parrtf \
    svtools/source/svrtf/rtfkeywd \
    svtools/source/svrtf/rtfout \
    svtools/source/svrtf/svparser \
    svtools/source/table/defaultinputhandler \
    svtools/source/table/gridtablerenderer \
    svtools/source/table/tablecontrol \
    svtools/source/table/tablecontrol_impl \
    svtools/source/table/tabledatawindow \
    svtools/source/table/mousefunction \
    svtools/source/table/cellvalueconversion \
    svtools/source/table/tablegeometry \
    svtools/source/uno/addrtempuno \
    svtools/source/uno/fpicker \
    svtools/source/uno/framestatuslistener \
    svtools/source/uno/genericunodialog \
    svtools/source/uno/miscservices \
    svtools/source/uno/popupmenucontrollerbase \
    svtools/source/uno/popupwindowcontroller \
    svtools/source/uno/statusbarcontroller \
    svtools/source/uno/svtxgridcontrol \
    svtools/source/uno/toolboxcontroller \
    svtools/source/uno/unocontroltablemodel \
    svtools/source/uno/unogridcolumnfacade \
    svtools/source/uno/unoevent \
    svtools/source/uno/unoiface \
    svtools/source/uno/unoimap \
    svtools/source/uno/wizard/unowizard \
    svtools/source/uno/wizard/wizardpagecontroller \
    svtools/source/uno/wizard/wizardshell \
    svtools/source/uitest/uiobject \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,svt,\
    advapi32 \
    gdi32 \
    ole32 \
    oleaut32 \
    uuid \
))
endif

# vim: set noet sw=4 ts=4:
