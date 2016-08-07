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



$(eval $(call gb_Package_Package,sd_uiconfig,$(SRCDIR)/sd/uiconfig))

$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/menubar/menubar.xml,simpress/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/3dobjectsbar.xml,simpress/toolbar/3dobjectsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/alignmentbar.xml,simpress/toolbar/alignmentbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/arrowsbar.xml,simpress/toolbar/arrowsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/arrowshapes.xml,simpress/toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/basicshapes.xml,simpress/toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/bezierobjectbar.xml,simpress/toolbar/bezierobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/calloutshapes.xml,simpress/toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/choosemodebar.xml,simpress/toolbar/choosemodebar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/colorbar.xml,simpress/toolbar/colorbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/commentsbar.xml,simpress/toolbar/commentsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/commontaskbar.xml,simpress/toolbar/commontaskbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/connectorsbar.xml,simpress/toolbar/connectorsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/drawingobjectbar.xml,simpress/toolbar/drawingobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/ellipsesbar.xml,simpress/toolbar/ellipsesbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/extrusionobjectbar.xml,simpress/toolbar/extrusionobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/findbar.xml,simpress/toolbar/findbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/flowchartshapes.xml,simpress/toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/fontworkobjectbar.xml,simpress/toolbar/fontworkobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/fontworkshapetype.xml,simpress/toolbar/fontworkshapetype.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/formcontrols.xml,simpress/toolbar/formcontrols.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/formdesign.xml,simpress/toolbar/formdesign.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/formsfilterbar.xml,simpress/toolbar/formsfilterbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/formsnavigationbar.xml,simpress/toolbar/formsnavigationbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/formtextobjectbar.xml,simpress/toolbar/formtextobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/fullscreenbar.xml,simpress/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/gluepointsobjectbar.xml,simpress/toolbar/gluepointsobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/graffilterbar.xml,simpress/toolbar/graffilterbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/graphicobjectbar.xml,simpress/toolbar/graphicobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/insertbar.xml,simpress/toolbar/insertbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/linesbar.xml,simpress/toolbar/linesbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/masterviewtoolbar.xml,simpress/toolbar/masterviewtoolbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/mediaobjectbar.xml,simpress/toolbar/mediaobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/moreformcontrols.xml,simpress/toolbar/moreformcontrols.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/optimizetablebar.xml,simpress/toolbar/optimizetablebar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/optionsbar.xml,simpress/toolbar/optionsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/outlinetoolbar.xml,simpress/toolbar/outlinetoolbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/positionbar.xml,simpress/toolbar/positionbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/rectanglesbar.xml,simpress/toolbar/rectanglesbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/slideviewobjectbar.xml,simpress/toolbar/slideviewobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/slideviewtoolbar.xml,simpress/toolbar/slideviewtoolbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/standardbar.xml,simpress/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/starshapes.xml,simpress/toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/symbolshapes.xml,simpress/toolbar/symbolshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/tableobjectbar.xml,simpress/toolbar/tableobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/textbar.xml,simpress/toolbar/textbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/textobjectbar.xml,simpress/toolbar/textobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/toolbar.xml,simpress/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/viewerbar.xml,simpress/toolbar/viewerbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/toolbar/zoombar.xml,simpress/toolbar/zoombar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/simpress/statusbar/statusbar.xml,simpress/statusbar/statusbar.xml))

$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/menubar/menubar.xml,sdraw/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/3dobjectsbar.xml,sdraw/toolbar/3dobjectsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/commentsbar.xml,sdraw/toolbar/commentsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/formcontrols.xml,sdraw/toolbar/formcontrols.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/insertbar.xml,sdraw/toolbar/insertbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/standardbar.xml,sdraw/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/alignmentbar.xml,sdraw/toolbar/alignmentbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/connectorsbar.xml,sdraw/toolbar/connectorsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/formdesign.xml,sdraw/toolbar/formdesign.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/linesbar.xml,sdraw/toolbar/linesbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/starshapes.xml,sdraw/toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/arrowsbar.xml,sdraw/toolbar/arrowsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/drawingobjectbar.xml,sdraw/toolbar/drawingobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/formsfilterbar.xml,sdraw/toolbar/formsfilterbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/masterviewtoolbar.xml,sdraw/toolbar/masterviewtoolbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/symbolshapes.xml,sdraw/toolbar/symbolshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/arrowshapes.xml,sdraw/toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/ellipsesbar.xml,sdraw/toolbar/ellipsesbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/formsnavigationbar.xml,sdraw/toolbar/formsnavigationbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/mediaobjectbar.xml,sdraw/toolbar/mediaobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/tableobjectbar.xml,sdraw/toolbar/tableobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/basicshapes.xml,sdraw/toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/extrusionobjectbar.xml,sdraw/toolbar/extrusionobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/formtextobjectbar.xml,sdraw/toolbar/formtextobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/moreformcontrols.xml,sdraw/toolbar/moreformcontrols.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/textbar.xml,sdraw/toolbar/textbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/bezierobjectbar.xml,sdraw/toolbar/bezierobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/findbar.xml,sdraw/toolbar/findbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/fullscreenbar.xml,sdraw/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/optimizetablebar.xml,sdraw/toolbar/optimizetablebar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/textobjectbar.xml,sdraw/toolbar/textobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/calloutshapes.xml,sdraw/toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/flowchartshapes.xml,sdraw/toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/gluepointsobjectbar.xml,sdraw/toolbar/gluepointsobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/optionsbar.xml,sdraw/toolbar/optionsbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/toolbar.xml,sdraw/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/choosemodebar.xml,sdraw/toolbar/choosemodebar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/fontworkobjectbar.xml,sdraw/toolbar/fontworkobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/graffilterbar.xml,sdraw/toolbar/graffilterbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/positionbar.xml,sdraw/toolbar/positionbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/viewerbar.xml,sdraw/toolbar/viewerbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/colorbar.xml,sdraw/toolbar/colorbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/fontworkshapetype.xml,sdraw/toolbar/fontworkshapetype.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/graphicobjectbar.xml,sdraw/toolbar/graphicobjectbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/rectanglesbar.xml,sdraw/toolbar/rectanglesbar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/toolbar/zoombar.xml,sdraw/toolbar/zoombar.xml))
$(eval $(call gb_Package_add_file,sd_uiconfig,xml/uiconfig/modules/sdraw/statusbar/statusbar.xml,sdraw/statusbar/statusbar.xml))
