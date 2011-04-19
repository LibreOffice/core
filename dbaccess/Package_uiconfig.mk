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

$(eval $(call gb_Package_Package,dbaccess_uiconfig,$(SRCDIR)/dbaccess/uiconfig))

$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/menubar/menubar.xml,dbapp/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtdata/menubar/menubar.xml,dbtdata/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/menubar/menubar.xml,dbquery/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbrelation/menubar/menubar.xml,dbrelation/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtable/menubar/menubar.xml,dbtable/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/formobjectbar.xml,dbapp/toolbar/formobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/queryobjectbar.xml,dbapp/toolbar/queryobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/reportobjectbar.xml,dbapp/toolbar/reportobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/tableobjectbar.xml,dbapp/toolbar/tableobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/toolbar.xml,dbapp/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/statusbar/statusbar.xml,dbapp/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbbrowser/menubar/compat.xml,dbbrowser/menubar/compat.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbbrowser/toolbar/toolbar.xml,dbbrowser/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtdata/toolbar/toolbar.xml,dbtdata/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/toolbar/designobjectbar.xml,dbquery/toolbar/designobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/toolbar/sqlobjectbar.xml,dbquery/toolbar/sqlobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/toolbar/toolbar.xml,dbquery/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbrelation/toolbar/toolbar.xml,dbrelation/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtable/toolbar/toolbar.xml,dbtable/toolbar/toolbar.xml))
