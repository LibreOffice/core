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

$(eval $(call gb_Package_Package,toolkit_inc,$(SRCDIR)/toolkit/inc))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/layout/layout-post.hxx,layout/layout-post.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/layout/layout-pre.hxx,layout/layout-pre.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/layout/layout.hxx,layout/layout.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxaccessiblecomponent.hxx,toolkit/awt/vclxaccessiblecomponent.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxcontainer.hxx,toolkit/awt/vclxcontainer.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxdevice.hxx,toolkit/awt/vclxdevice.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxfont.hxx,toolkit/awt/vclxfont.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxmenu.hxx,toolkit/awt/vclxmenu.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxsystemdependentwindow.hxx,toolkit/awt/vclxsystemdependentwindow.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxtoolkit.hxx,toolkit/awt/vclxtoolkit.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxtopwindow.hxx,toolkit/awt/vclxtopwindow.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxwindow.hxx,toolkit/awt/vclxwindow.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/awt/vclxwindows.hxx,toolkit/awt/vclxwindows.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/controls/unocontrol.hxx,toolkit/controls/unocontrol.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/controls/unocontrolbase.hxx,toolkit/controls/unocontrolbase.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/controls/unocontrolmodel.hxx,toolkit/controls/unocontrolmodel.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/controls/unocontrols.hxx,toolkit/controls/unocontrols.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/dllapi.h,toolkit/dllapi.h))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/accessibilityclient.hxx,toolkit/helper/accessibilityclient.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/accessiblefactory.hxx,toolkit/helper/accessiblefactory.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/convert.hxx,toolkit/helper/convert.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/emptyfontdescriptor.hxx,toolkit/helper/emptyfontdescriptor.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/externallock.hxx,toolkit/helper/externallock.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/fixedhyperbase.hxx,toolkit/helper/fixedhyperbase.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/formpdfexport.hxx,toolkit/helper/formpdfexport.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/imagealign.hxx,toolkit/helper/imagealign.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/listenermultiplexer.hxx,toolkit/helper/listenermultiplexer.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/macros.hxx,toolkit/helper/macros.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/mutexandbroadcasthelper.hxx,toolkit/helper/mutexandbroadcasthelper.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/mutexhelper.hxx,toolkit/helper/mutexhelper.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/property.hxx,toolkit/helper/property.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/servicenames.hxx,toolkit/helper/servicenames.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/tkresmgr.hxx,toolkit/helper/tkresmgr.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/unomemorystream.hxx,toolkit/helper/unomemorystream.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/unopropertyarrayhelper.hxx,toolkit/helper/unopropertyarrayhelper.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/unowrapper.hxx,toolkit/helper/unowrapper.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/helper/vclunohelper.hxx,toolkit/helper/vclunohelper.hxx))
$(eval $(call gb_Package_add_file,toolkit_inc,inc/toolkit/unohlp.hxx,toolkit/helper/vclunohelper.hxx))
