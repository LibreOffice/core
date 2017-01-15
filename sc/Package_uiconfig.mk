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



$(eval $(call gb_Package_Package,sc_uiconfig,$(SRCDIR)/sc/uiconfig))

$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/menubar/menubar.xml,scalc/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/graffilterbar.xml,scalc/toolbar/graffilterbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/textobjectbar.xml,scalc/toolbar/textobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/fullscreenbar.xml,scalc/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/arrowshapes.xml,scalc/toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/previewbar.xml,scalc/toolbar/previewbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formsfilterbar.xml,scalc/toolbar/formsfilterbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/drawobjectbar.xml,scalc/toolbar/drawobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formatobjectbar.xml,scalc/toolbar/formatobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/symbolshapes.xml,scalc/toolbar/symbolshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/mediaobjectbar.xml,scalc/toolbar/mediaobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formsnavigationbar.xml,scalc/toolbar/formsnavigationbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/starshapes.xml,scalc/toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/insertcellsbar.xml,scalc/toolbar/insertcellsbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/graphicobjectbar.xml,scalc/toolbar/graphicobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formtextobjectbar.xml,scalc/toolbar/formtextobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formcontrols.xml,scalc/toolbar/formcontrols.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formdesign.xml,scalc/toolbar/formdesign.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/fontworkshapetype.xml,scalc/toolbar/fontworkshapetype.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/colorbar.xml,scalc/toolbar/colorbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/standardbar.xml,scalc/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/toolbar.xml,scalc/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/alignmentbar.xml,scalc/toolbar/alignmentbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/extrusionobjectbar.xml,scalc/toolbar/extrusionobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/findbar.xml,scalc/toolbar/findbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/insertbar.xml,scalc/toolbar/insertbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/flowchartshapes.xml,scalc/toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/viewerbar.xml,scalc/toolbar/viewerbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/drawbar.xml,scalc/toolbar/drawbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/basicshapes.xml,scalc/toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/fontworkobjectbar.xml,scalc/toolbar/fontworkobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/moreformcontrols.xml,scalc/toolbar/moreformcontrols.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/calloutshapes.xml,scalc/toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/statusbar/statusbar.xml,scalc/statusbar/statusbar.xml))
