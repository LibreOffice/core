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

    # In this view, the sections "SectionB" and "SectionC" on second page are positioned on screen
    # higher than "SectionY" and "SectionA" respectively; there are nested and anchored sections.
    # Make sure that order in Navigator follows their relative position in document, not vertical
    # position on screen, nor sorted alphabetically. Sections in flying frames are sorted by their
    # anchor position in the document.

    self.xUITest.executeCommand(".uno:Sidebar")
    xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

    # wait until the navigator panel is available
    xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanelParent')

    xContentTree = xNavigatorPanel.getChild("contenttree")
    xSections = xContentTree.getChild('6')
    self.assertEqual('Sections', get_state_as_dict(xSections)['Text'])
    xSections.executeAction("EXPAND", ())

    refSectionNames = [
      'SectionZ',
      'SectionY', # SectionB should not get before this, despite its Y position on screen is higher
      'SectionT3', # Sections in tables go in rows, then across rows
      'SectionT1',
      'SectionT2',
      'SectionT0',
      'SectionF2', # Goes before SectionF1, because their fly anchors go in that order
      'SectionF3', # Same as SectionF1, but anchor section is in fly itself
      'SectionFinF3', # Check order of nested sections inside fly
      'SectionA',
      'SectionF1', # Section in fly anchored in a section goes immediately after its anchor section
      'SectionB', # High on screen, but late in list because it's on second page
      'SectionC',
    ]
    self.assertEqual(len(refSectionNames), len(xSections.getChildren()))

    actSectionNames = []
    for i in range(len(refSectionNames)):
      actSectionNames.append(get_state_as_dict(xSections.getChild(str(i)))['Text'])
    # Without the fix in place, this would fail with
    #   AssertionError: Lists differ: ['SectionZ', 'SectionY', 'SectionT3', 'SectionT1', 'SectionT2'[100 chars]onC'] != ['SectionZ', 'SectionB', 'SectionF3', 'SectionFinF3', 'Section[100 chars]onA']
    self.assertEqual(refSectionNames, actSectionNames)

    self.xUITest.executeCommand(".uno:Sidebar")
    self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
