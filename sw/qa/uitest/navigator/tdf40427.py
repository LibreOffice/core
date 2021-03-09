# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class tdf40427(UITestCase):

  def test_tdf40427(self):
    self.ui_test.load_file(get_url_for_data_file("tdf40427_SectionPositions.odt"))
    xMainWindow = self.xUITest.getTopFocusWindow()
    xWriterEdit = xMainWindow.getChild("writer_edit")

    self.assertEqual(2, self.ui_test.get_component().CurrentController.PageCount)

    # Make sure that the view is 2 pages side-by-side - look at dialog View-Zoom-Zoom
    self.ui_test.execute_dialog_through_command(".uno:Zoom")
    xDialog = self.xUITest.getTopFocusWindow()

    columnssb = xDialog.getChild("columnssb")
    columns = xDialog.getChild("columns")
    bookmode = xDialog.getChild("bookmode")
    self.assertEqual("true", get_state_as_dict(columns)["Checked"])
    self.assertEqual("2", get_state_as_dict(columnssb)["Text"])
    self.assertEqual("false", get_state_as_dict(bookmode)["Selected"])

    xOKBtn = xDialog.getChild("ok")
    self.ui_test.close_dialog_through_button(xOKBtn)

    # In this view, the sections "SectionB" and "SectionC" on second page are positioned
    # on screen higher than "SectionY" and "SectionA" respectively. Make sure that order
    # in Navigator follows their relative position in document, not vertical position on
    # screen, nor sorted alphabetically

    self.xUITest.executeCommand(".uno:Sidebar")
    xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

    # wait until the navigator panel is available
    xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanelParent')

    xContentTree = xNavigatorPanel.getChild("contenttree")
    xSections = xContentTree.getChild('6')
    self.assertEqual('Sections', get_state_as_dict(xSections)['Text'])

    xSections.executeAction("EXPAND", tuple())

    self.assertEqual(5, len(xSections.getChildren()))

    sectionNames = []
    for i in range(5):
      sectionNames.append(get_state_as_dict(xSections.getChild(str(i)))['Text'])
    self.assertEqual(['SectionZ', 'SectionY', 'SectionA', 'SectionB', 'SectionC'], sectionNames)

    self.xUITest.executeCommand(".uno:Sidebar")
    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
