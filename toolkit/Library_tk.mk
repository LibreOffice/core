#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,tk))

$(eval $(call gb_Library_set_componentfile,tk,toolkit/util/tk))

$(eval $(call gb_Library_add_package_headers,tk,toolkit_inc))

#$(eval $(call gb_Library_add_precompiled_header,tk,$(SRCDIR)/toolkit/inc/pch/precompiled_toolkit))

$(eval $(call gb_Library_set_include,tk,\
    $$(INCLUDE) \
    -I$(SRCDIR)/toolkit/inc \
    -I$(SRCDIR)/toolkit/inc/pch \
    -I$(SRCDIR)/toolkit/source \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,tk,\
    $$(DEFS) \
    -DTOOLKIT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,tk,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    stl \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,tk,\
    toolkit/source/awt/asynccallback \
    toolkit/source/awt/stylesettings \
    toolkit/source/awt/vclxaccessiblecomponent \
    toolkit/source/awt/vclxbitmap \
    toolkit/source/awt/vclxbutton \
    toolkit/source/awt/vclxcontainer \
    toolkit/source/awt/vclxdevice \
    toolkit/source/awt/vclxdialog \
    toolkit/source/awt/vclxfixedline \
    toolkit/source/awt/vclxfont \
    toolkit/source/awt/vclxgraphics \
    toolkit/source/awt/vclxmenu \
    toolkit/source/awt/vclxplugin \
    toolkit/source/awt/vclxpointer \
    toolkit/source/awt/vclxprinter \
    toolkit/source/awt/vclxregion \
    toolkit/source/awt/vclxscroller \
    toolkit/source/awt/vclxspinbutton \
    toolkit/source/awt/vclxsplitter \
    toolkit/source/awt/vclxsystemdependentwindow \
    toolkit/source/awt/vclxtabcontrol \
    toolkit/source/awt/vclxtabpage \
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
    toolkit/source/controls/tabpagecontainer \
    toolkit/source/controls/tabpagemodel \
    toolkit/source/controls/stdtabcontroller \
    toolkit/source/controls/stdtabcontrollermodel \
    toolkit/source/controls/tkscrollbar \
    toolkit/source/controls/tkspinbutton \
    toolkit/source/controls/animatedimages \
    toolkit/source/controls/spinningprogress \
    toolkit/source/controls/tree/treecontrol \
    toolkit/source/controls/tree/treedatamodel \
    toolkit/source/controls/unocontrol \
    toolkit/source/controls/unocontrolbase \
    toolkit/source/controls/unocontrolcontainer \
    toolkit/source/controls/unocontrolcontainermodel \
    toolkit/source/controls/unocontrolmodel \
    toolkit/source/controls/unocontrols \
    toolkit/source/helper/accessibilityclient \
    toolkit/source/helper/externallock \
    toolkit/source/helper/fixedhyperbase \
    toolkit/source/helper/formpdfexport \
    toolkit/source/helper/imagealign \
    toolkit/source/helper/listenermultiplexer \
    toolkit/source/helper/property \
    toolkit/source/helper/registerservices \
    toolkit/source/helper/servicenames \
    toolkit/source/helper/tkresmgr \
    toolkit/source/helper/unomemorystream \
    toolkit/source/helper/unopropertyarrayhelper \
    toolkit/source/helper/unowrapper \
    toolkit/source/helper/vclunohelper \
    toolkit/source/layout/core/bin \
    toolkit/source/layout/core/box \
    toolkit/source/layout/core/box-base \
    toolkit/source/layout/core/byteseq \
    toolkit/source/layout/core/container \
    toolkit/source/layout/core/dialogbuttonhbox \
    toolkit/source/layout/core/factory \
    toolkit/source/layout/core/flow \
    toolkit/source/layout/core/helper \
    toolkit/source/layout/core/import \
    toolkit/source/layout/core/localized-string \
    toolkit/source/layout/core/proplist \
    toolkit/source/layout/core/root \
    toolkit/source/layout/core/table \
    toolkit/source/layout/core/timer \
    toolkit/source/layout/core/translate \
    toolkit/source/layout/core/vcl \
    toolkit/source/layout/vcl/wbutton \
    toolkit/source/layout/vcl/wcontainer \
    toolkit/source/layout/vcl/wfield \
    toolkit/source/layout/vcl/wrapper \
))

ifeq ($(GUIBASE),aqua)
$(eval $(call gb_Library_set_cxxflags,tk,\
    $$(CXXFLAGS) $(gb_OBJCXXFLAGS)))
endif

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,tk,\
    X11 \
))
endif

# vim: set noet sw=4 ts=4:
