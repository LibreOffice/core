###############################################################
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
###############################################################



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
