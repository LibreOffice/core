# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,sc_uiconfig,$(SRCDIR)/sc/uiconfig))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/layout/insert-sheet.xml,layout/insert-sheet.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/layout/move-copy-sheet.xml,layout/move-copy-sheet.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/layout/sort-options.xml,layout/sort-options.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/layout/string-input.xml,layout/string-input.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/menubar/menubar.xml,scalc/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/alignmentbar.xml,scalc/toolbar/alignmentbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/arrowshapes.xml,scalc/toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/basicshapes.xml,scalc/toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/calloutshapes.xml,scalc/toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/colorbar.xml,scalc/toolbar/colorbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/drawbar.xml,scalc/toolbar/drawbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/drawobjectbar.xml,scalc/toolbar/drawobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/extrusionobjectbar.xml,scalc/toolbar/extrusionobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/findbar.xml,scalc/toolbar/findbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/flowchartshapes.xml,scalc/toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/fontworkobjectbar.xml,scalc/toolbar/fontworkobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/fontworkshapetype.xml,scalc/toolbar/fontworkshapetype.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formatobjectbar.xml,scalc/toolbar/formatobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formcontrols.xml,scalc/toolbar/formcontrols.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formdesign.xml,scalc/toolbar/formdesign.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formsfilterbar.xml,scalc/toolbar/formsfilterbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formsnavigationbar.xml,scalc/toolbar/formsnavigationbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/formtextobjectbar.xml,scalc/toolbar/formtextobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/fullscreenbar.xml,scalc/toolbar/fullscreenbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/graffilterbar.xml,scalc/toolbar/graffilterbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/graphicobjectbar.xml,scalc/toolbar/graphicobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/insertbar.xml,scalc/toolbar/insertbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/insertcellsbar.xml,scalc/toolbar/insertcellsbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/mediaobjectbar.xml,scalc/toolbar/mediaobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/moreformcontrols.xml,scalc/toolbar/moreformcontrols.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/previewbar.xml,scalc/toolbar/previewbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/standardbar.xml,scalc/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/starshapes.xml,scalc/toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/statusbar.xml,scalc/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/symbolshapes.xml,scalc/toolbar/symbolshapes.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/textobjectbar.xml,scalc/toolbar/textobjectbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/toolbar.xml,scalc/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,sc_uiconfig,xml/uiconfig/modules/scalc/toolbar/viewerbar.xml,scalc/toolbar/viewerbar.xml))

