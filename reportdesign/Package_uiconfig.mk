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
