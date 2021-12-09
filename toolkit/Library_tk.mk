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

$(eval $(call gb_Library_Library,tk))

$(eval $(call gb_Library_set_componentfile,tk,toolkit/util/tk,services))

$(eval $(call gb_Library_use_externals,tk,\
    boost_headers \
))

ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Library_use_externals,tk,\
    epoxy \
))
endif
$(eval $(call gb_Library_set_include,tk,\
    $$(INCLUDE) \
    -I$(SRCDIR)/toolkit/inc \
    -I$(SRCDIR)/toolkit/source \
))

$(eval $(call gb_Library_add_defs,tk,\
    -DTOOLKIT_DLLIMPLEMENTATION \
	-DSVT_DLL_NAME=\"$(call gb_Library_get_runtime_filename,$(call gb_Library__get_name,svt))\" \
))

$(eval $(call gb_Library_use_sdk_api,tk))

$(eval $(call gb_Library_use_libraries,tk,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
	i18nlangtag \
    svl \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_add_exception_objects,tk,\
    toolkit/source/awt/asynccallback \
    toolkit/source/awt/stylesettings \
    toolkit/source/awt/scrollabledialog \
    toolkit/source/awt/vclxaccessiblecomponent \
    toolkit/source/awt/vclxbitmap \
    toolkit/source/awt/vclxcontainer \
    toolkit/source/awt/vclxdevice \
    toolkit/source/awt/vclxfont \
    toolkit/source/awt/vclxgraphics \
    toolkit/source/awt/vclxmenu \
    toolkit/source/awt/vclxpointer \
    toolkit/source/awt/vclxprinter \
    toolkit/source/awt/vclxregion \
    toolkit/source/awt/vclxspinbutton \
    toolkit/source/awt/vclxsystemdependentwindow \
    toolkit/source/awt/vclxtabpagecontainer \
    toolkit/source/awt/animatedimagespeer \
    toolkit/source/awt/vclxtoolkit \
    toolkit/source/awt/vclxtopwindow \
    toolkit/source/awt/vclxwindow \
    toolkit/source/awt/vclxwindow1 \
    toolkit/source/awt/vclxwindows \
    toolkit/source/controls/accessiblecontrolcontext \
    toolkit/source/controls/controlmodelcontainerbase \
    toolkit/source/controls/dialogcontrol \
    toolkit/source/controls/eventcontainer \
    toolkit/source/controls/filectrl \
    toolkit/source/controls/formattedcontrol \
    toolkit/source/controls/geometrycontrolmodel \
    toolkit/source/controls/grid/defaultgridcolumnmodel \
    toolkit/source/controls/grid/defaultgriddatamodel \
    toolkit/source/controls/grid/gridcolumn \
    toolkit/source/controls/grid/grideventforwarder \
    toolkit/source/controls/grid/sortablegriddatamodel \
    toolkit/source/controls/grid/gridcontrol \
    toolkit/source/controls/roadmapcontrol \
    toolkit/source/controls/roadmapentry \
    toolkit/source/controls/svmedit \
    toolkit/source/controls/tabpagecontainer \
    toolkit/source/controls/tabpagemodel \
    toolkit/source/controls/stdtabcontroller \
    toolkit/source/controls/stdtabcontrollermodel \
    toolkit/source/controls/tkscrollbar \
    toolkit/source/controls/tkspinbutton \
    toolkit/source/controls/animatedimages \
    toolkit/source/controls/spinningprogress \
    toolkit/source/controls/tree/treecontrol \
    toolkit/source/controls/tree/treecontrolpeer \
    toolkit/source/controls/tree/treedatamodel \
    toolkit/source/controls/unocontrol \
    toolkit/source/controls/unocontrolbase \
    toolkit/source/controls/unocontrolcontainer \
    toolkit/source/controls/unocontrolcontainermodel \
    toolkit/source/controls/unocontrolmodel \
    toolkit/source/controls/unocontrols \
    toolkit/source/hatchwindow/documentcloser \
    toolkit/source/hatchwindow/hatchwindow \
    toolkit/source/hatchwindow/hatchwindowfactory \
    toolkit/source/hatchwindow/ipwin \
    toolkit/source/helper/accessibilityclient \
    toolkit/source/helper/btndlg \
    toolkit/source/helper/formpdfexport \
    toolkit/source/helper/imagealign \
    toolkit/source/helper/listenermultiplexer \
    toolkit/source/helper/property \
    toolkit/source/helper/servicenames \
    toolkit/source/helper/tkresmgr \
    toolkit/source/helper/unopropertyarrayhelper \
    toolkit/source/helper/unowrapper \
    toolkit/source/helper/vclunohelper \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,tk,\
	$(gb_OBJCXXFLAGS) \
))
$(eval $(call gb_Library_add_libs,tk,\
	-lobjc \
))
endif

ifeq ($(OS),iOS)
$(eval $(call gb_Library_add_cxxflags,tk,\
    $(gb_OBJCXXFLAGS)))
endif

# vim: set noet sw=4 ts=4:
