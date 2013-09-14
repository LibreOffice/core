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

$(eval $(call gb_Library_Library,sfx))

$(eval $(call gb_Library_add_sdi_headers,sfx,sfx2/sdi/sfxslots))

$(eval $(call gb_Library_set_componentfile,sfx,sfx2/util/sfx))

$(eval $(call gb_Library_set_precompiled_header,sfx,$(SRCDIR)/sfx2/inc/pch/precompiled_sfx))

$(eval $(call gb_Library_use_custom_headers,sfx,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,sfx))

$(eval $(call gb_Library_set_include,sfx,\
    -I$(SRCDIR)/sfx2/inc \
    -I$(SRCDIR)/sfx2/source/inc \
    -I$(WORKDIR)/SdiTarget/sfx2/sdi \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,sfx,-DSFX2_DLLIMPLEMENTATION))

$(eval $(call gb_Library_add_defs,sfx,\
    $(if $(filter TRUE,$(ENABLE_CUPS)),-DENABLE_CUPS) \
))

ifeq ($(ENABLE_SYSTRAY_GTK),TRUE)
$(eval $(call gb_Library_add_defs,sfx,\
    -DENABLE_QUICKSTART_APPLET \
    -DENABLE_SYSTRAY_GTK \
    -DPLUGIN_NAME=libqstart_gtk$(gb_Library_OOOEXT) \
))
endif

$(eval $(call gb_Library_use_libraries,sfx,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    fwe \
    i18nlangtag \
    sal \
    sax \
    sb \
    sot \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,sfx,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,sfx,\
    sfx2/source/appl/app \
    sfx2/source/appl/appbas \
    sfx2/source/appl/appbaslib \
    sfx2/source/appl/appcfg \
    sfx2/source/appl/appchild \
    sfx2/source/appl/appdata \
    sfx2/source/appl/appdde \
    sfx2/source/appl/appinit \
    sfx2/source/appl/appmain \
    sfx2/source/appl/appmisc \
    sfx2/source/appl/appopen \
    sfx2/source/appl/appquit \
    sfx2/source/appl/appreg \
    sfx2/source/appl/appserv \
    sfx2/source/appl/appuno \
    sfx2/source/appl/childwin \
    sfx2/source/appl/fileobj \
    sfx2/source/appl/fwkhelper \
    sfx2/source/appl/helpdispatch \
    sfx2/source/appl/helpinterceptor \
    sfx2/source/appl/imagemgr \
    sfx2/source/appl/imestatuswindow \
    sfx2/source/appl/impldde \
    sfx2/source/appl/linkmgr2 \
    sfx2/source/appl/linksrc \
    sfx2/source/appl/lnkbase2 \
    sfx2/source/appl/module \
    sfx2/source/appl/newhelp \
    sfx2/source/appl/opengrf \
    sfx2/source/appl/openuriexternally \
    sfx2/source/appl/sfxhelp \
    sfx2/source/appl/sfxpicklist \
    sfx2/source/appl/shutdownicon \
    sfx2/source/appl/workwin \
    sfx2/source/appl/xpackcreator \
    sfx2/source/bastyp/bitset \
    sfx2/source/bastyp/fltfnc \
    sfx2/source/bastyp/fltlst \
    sfx2/source/bastyp/frmhtml \
    sfx2/source/bastyp/frmhtmlw \
    sfx2/source/bastyp/helper \
    sfx2/source/bastyp/mieclip \
    sfx2/source/bastyp/progress \
    sfx2/source/bastyp/sfxhtml \
    sfx2/source/bastyp/sfxresid \
    sfx2/source/config/evntconf \
    sfx2/source/control/bindings \
    sfx2/source/control/ctrlitem \
    sfx2/source/control/minfitem \
    sfx2/source/control/msg \
    sfx2/source/control/msgpool \
    sfx2/source/control/objface \
    sfx2/source/control/querystatus \
    sfx2/source/control/recentdocsview \
    sfx2/source/control/recentdocsviewitem \
    sfx2/source/control/request \
    sfx2/source/control/sfxstatuslistener \
    sfx2/source/control/shell \
    sfx2/source/control/sorgitm \
    sfx2/source/control/statcach \
    sfx2/source/control/templateabstractview \
    sfx2/source/control/templateviewitem \
    sfx2/source/control/templatelocalview \
    sfx2/source/control/templatecontaineritem \
    sfx2/source/control/templateremoteview \
    sfx2/source/control/templatesearchview \
    sfx2/source/control/thumbnailviewitem \
    sfx2/source/control/thumbnailviewacc \
    sfx2/source/control/thumbnailview \
    sfx2/source/control/unoctitm \
    sfx2/source/dialog/alienwarn \
    sfx2/source/dialog/backingcomp \
    sfx2/source/dialog/backingwindow \
    sfx2/source/dialog/basedlgs \
    sfx2/source/dialog/checkin \
    sfx2/source/dialog/dialoghelper \
    sfx2/source/dialog/dinfdlg \
    sfx2/source/dialog/dinfedt \
    sfx2/source/dialog/dockwin \
    sfx2/source/dialog/documentfontsdialog \
    sfx2/source/dialog/filedlghelper \
    sfx2/source/dialog/filtergrouping \
    sfx2/source/dialog/itemconnect \
    sfx2/source/dialog/infobar \
    sfx2/source/dialog/inputdlg \
    sfx2/source/dialog/mailmodel \
    sfx2/source/dialog/bluthsnd \
    sfx2/source/dialog/mgetempl \
    sfx2/source/dialog/navigat \
    sfx2/source/dialog/newstyle \
    sfx2/source/dialog/partwnd \
    sfx2/source/dialog/passwd \
    sfx2/source/dialog/printopt \
    sfx2/source/dialog/recfloat \
    sfx2/source/dialog/securitypage \
    sfx2/source/dialog/sfxdlg \
    sfx2/source/dialog/splitwin \
    sfx2/source/dialog/srchdlg \
    sfx2/source/dialog/styfitem \
    sfx2/source/dialog/styledlg \
    sfx2/source/dialog/tabdlg \
    sfx2/source/dialog/taskpane \
    sfx2/source/dialog/templateinfodlg \
    sfx2/source/dialog/templdlg \
    sfx2/source/dialog/titledockwin \
    sfx2/source/dialog/tplcitem \
    sfx2/source/dialog/tplpitem \
    sfx2/source/dialog/versdlg \
    sfx2/source/doc/DocumentMetadataAccess \
    sfx2/source/doc/Metadatable \
    sfx2/source/doc/QuerySaveDocument \
    sfx2/source/doc/SfxDocumentMetaData \
    sfx2/source/doc/docfac \
    sfx2/source/doc/docfile \
    sfx2/source/doc/docfilt \
    sfx2/source/doc/docinf \
    sfx2/source/doc/docinsert \
    sfx2/source/doc/docmacromode \
    sfx2/source/doc/docstoragemodifylistener \
    sfx2/source/doc/doctempl \
    sfx2/source/doc/doctemplates \
    sfx2/source/doc/doctemplateslocal \
    sfx2/source/doc/frmdescr \
    sfx2/source/doc/graphhelp \
    sfx2/source/doc/guisaveas \
    sfx2/source/doc/iframe \
    sfx2/source/doc/new \
    sfx2/source/doc/objcont \
    sfx2/source/doc/objembed \
    sfx2/source/doc/objitem \
    sfx2/source/doc/objmisc \
    sfx2/source/doc/objserv \
    sfx2/source/doc/objstor \
    sfx2/source/doc/objxtor \
    sfx2/source/doc/oleprops \
    sfx2/source/doc/ownsubfilterservice \
    sfx2/source/doc/plugin \
    sfx2/source/doc/printhelper \
    sfx2/source/doc/querytemplate \
    sfx2/source/doc/docundomanager \
    sfx2/source/doc/sfxacldetect \
    sfx2/source/doc/sfxbasemodel \
    sfx2/source/doc/sfxmodelfactory \
    sfx2/source/doc/syspath \
    sfx2/source/doc/zoomitem \
    sfx2/source/doc/templatedlg \
    sfx2/source/explorer/nochaos \
    sfx2/source/inet/inettbc \
    sfx2/source/menu/mnuitem \
    sfx2/source/menu/mnumgr \
    sfx2/source/menu/thessubmenu \
    sfx2/source/menu/virtmenu \
    sfx2/source/notify/eventsupplier \
    sfx2/source/notify/hintpost \
    sfx2/source/sidebar/Sidebar \
    sfx2/source/sidebar/SidebarChildWindow \
    sfx2/source/sidebar/SidebarDockingWindow \
    sfx2/source/sidebar/SidebarController \
    sfx2/source/sidebar/SidebarPanelBase \
    sfx2/source/sidebar/SidebarToolBox \
    sfx2/source/sidebar/Accessible \
    sfx2/source/sidebar/AccessibleTitleBar \
    sfx2/source/sidebar/AsynchronousCall \
    sfx2/source/sidebar/CommandInfoProvider \
    sfx2/source/sidebar/Context \
    sfx2/source/sidebar/ContextChangeBroadcaster \
    sfx2/source/sidebar/ContextList \
    sfx2/source/sidebar/ControlFactory \
    sfx2/source/sidebar/ControllerFactory \
    sfx2/source/sidebar/ControllerItem \
    sfx2/source/sidebar/Deck \
    sfx2/source/sidebar/DeckDescriptor \
    sfx2/source/sidebar/DeckLayouter \
    sfx2/source/sidebar/DeckTitleBar \
    sfx2/source/sidebar/DrawHelper \
    sfx2/source/sidebar/EnumContext \
    sfx2/source/sidebar/FocusManager \
    sfx2/source/sidebar/MenuButton \
    sfx2/source/sidebar/IContextChangeReceiver \
    sfx2/source/sidebar/ILayoutableWindow \
    sfx2/source/sidebar/Paint \
    sfx2/source/sidebar/Panel \
    sfx2/source/sidebar/PanelDescriptor \
    sfx2/source/sidebar/PanelTitleBar \
    sfx2/source/sidebar/ResourceManager \
    sfx2/source/sidebar/TabBar \
    sfx2/source/sidebar/TabItem \
    sfx2/source/sidebar/TitleBar \
    sfx2/source/sidebar/Theme \
    sfx2/source/sidebar/Tools \
    sfx2/source/statbar/stbitem \
    sfx2/source/toolbox/imgmgr \
    sfx2/source/toolbox/tbxitem \
    sfx2/source/view/frame \
    sfx2/source/view/frame2 \
    sfx2/source/view/frmload \
    sfx2/source/view/ipclient \
    sfx2/source/view/printer \
    sfx2/source/view/sfxbasecontroller \
    sfx2/source/view/userinputinterception \
    sfx2/source/view/viewfac \
    sfx2/source/view/viewfrm \
    sfx2/source/view/viewfrm2 \
    sfx2/source/view/viewprn \
    sfx2/source/view/viewsh \
))

# i116803: crash in impress when
# "opening stylist, select graphic style, select modify from context menu, cancel dialog, close stylist"
# only on unxlngi6 and unxlngi6.pro platform
ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_cxxobjects,sfx,\
       sfx2/source/control/dispatch \
    , $(gb_COMPILERNOOPTFLAGS) $(if $(call gb_LinkTarget__debug_enabled,sfx),$(gb_DEBUG_CFLAGS)) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,sfx,\
       sfx2/source/control/dispatch \
))
endif


$(eval $(call gb_SdiTarget_SdiTarget,sfx2/sdi/sfxslots,sfx2/sdi/sfx))

$(eval $(call gb_SdiTarget_set_include,sfx2/sdi/sfxslots,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sfx2/inc \
    -I$(SRCDIR)/sfx2/sdi \
))

ifeq ($(OS),$(filter WNT MACOSX,$(OS)))
$(eval $(call gb_Library_add_defs,sfx,\
    -DENABLE_QUICKSTART_APPLET \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_objcxxobjects,sfx,\
    sfx2/source/appl/shutdowniconaqua \
))
$(eval $(call gb_Library_add_libs,sfx,\
    -lobjc \
))
$(eval $(call gb_Library_use_system_darwin_frameworks,sfx,\
    Cocoa \
))
endif

ifeq ($(OS),WNT)

$(eval $(call gb_Library_add_exception_objects,sfx,\
    sfx2/source/appl/shutdowniconw32 \
    sfx2/source/doc/syspathw32 \
))

$(eval $(call gb_Library_use_system_win32_libs,sfx,\
    advapi32 \
    gdi32 \
    ole32 \
    shell32 \
    uuid \
))

endif

# Runtime dependency for unit-tests
$(eval $(call gb_Library_use_restarget,sfx,sfx))

# vim: set noet sw=4 ts=4:
