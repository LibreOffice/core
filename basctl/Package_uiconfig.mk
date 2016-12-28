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



$(eval $(call gb_Package_Package,basctl_uiconfig,$(SRCDIR)/basctl/uiconfig))

$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/menubar/menubar.xml,basicide/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/toolbar/dialogbar.xml,basicide/toolbar/dialogbar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/toolbar/translationbar.xml,basicide/toolbar/translationbar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/toolbar/macrobar.xml,basicide/toolbar/macrobar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/toolbar/standardbar.xml,basicide/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/toolbar/fullscreenbar.xml,basicide/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/toolbar/insertcontrolsbar.xml,basicide/toolbar/insertcontrolsbar.xml))
$(eval $(call gb_Package_add_file,basctl_uiconfig,xml/uiconfig/modules/BasicIDE/statusbar/statusbar.xml,basicide/statusbar/statusbar.xml))
