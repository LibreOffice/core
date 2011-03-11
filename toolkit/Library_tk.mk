#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
    toolkit/source/awt/xsimpleanimation \
    toolkit/source/awt/xthrobber \
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
    toolkit/source/controls/tksimpleanimation \
    toolkit/source/controls/tkspinbutton \
    toolkit/source/controls/tkthrobber \
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
