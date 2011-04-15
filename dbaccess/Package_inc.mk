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

$(eval $(call gb_Package_Package,dbaccess_inc,$(SRCDIR)/dbaccess/inc))

$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/AsyncronousLink.hxx ,AsyncronousLink.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/IController.hxx ,IController.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/IReference.hxx ,IReference.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/ToolBoxHelper.hxx ,ToolBoxHelper.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/controllerframe.hxx ,controllerframe.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dataview.hxx ,dataview.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dbaundomanager.hxx ,dbaundomanager.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dbaccessdllapi.h ,dbaccessdllapi.h ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dbsubcomponentcontroller.hxx ,dbsubcomponentcontroller.hxx ))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/genericcontroller.hxx ,genericcontroller.hxx ))
