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



$(eval $(call gb_Package_Package,sm_uiconfig,$(SRCDIR)/starmath/uiconfig))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/menubar/menubar.xml,smath/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/statusbar/statusbar.xml,smath/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/toolbar/fullscreenbar.xml,smath/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/toolbar/standardbar.xml,smath/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/toolbar/toolbar.xml,smath/toolbar/toolbar.xml))
