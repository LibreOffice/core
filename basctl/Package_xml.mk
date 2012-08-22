# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Package_Package,basctl_xml,$(SRCDIR)/basctl))

$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/menubar/menubar.xml,uiconfig/basicide/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/statusbar/statusbar.xml,uiconfig/basicide/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/dialogbar.xml,uiconfig/basicide/toolbar/dialogbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/findbar.xml,uiconfig/basicide/toolbar/findbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/fullscreenbar.xml,uiconfig/basicide/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/insertcontrolsbar.xml,uiconfig/basicide/toolbar/insertcontrolsbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/formcontrolsbar.xml,uiconfig/basicide/toolbar/formcontrolsbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/macrobar.xml,uiconfig/basicide/toolbar/macrobar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/standardbar.xml,uiconfig/basicide/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,basctl_xml,xml/uiconfig/modules/BasicIDE/toolbar/translationbar.xml,uiconfig/basicide/toolbar/translationbar.xml))

# vim: set noet sw=4 ts=4:
