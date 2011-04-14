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

$(eval $(call gb_Package_Package,reportdesign_uiconfig,$(SRCDIR)/reportdesign/uiconfig))

$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/menubar/menubar.xml,dbreport/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/statusbar/statusbar.xml,dbreport/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/toolbar.xml,dbreport/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/Formatting.xml,dbreport/toolbar/Formatting.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/alignmentbar.xml,dbreport/toolbar/alignmentbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/arrowshapes.xml,dbreport/toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/basicshapes.xml,dbreport/toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/calloutshapes.xml,dbreport/toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/drawbar.xml,dbreport/toolbar/drawbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/flowchartshapes.xml,dbreport/toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/reportcontrols.xml,dbreport/toolbar/reportcontrols.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/resizebar.xml,dbreport/toolbar/resizebar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/sectionalignmentbar.xml,dbreport/toolbar/sectionalignmentbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/sectionshrinkbar.xml,dbreport/toolbar/sectionshrinkbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/starshapes.xml,dbreport/toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/symbolshapes.xml,dbreport/toolbar/symbolshapes.xml))
