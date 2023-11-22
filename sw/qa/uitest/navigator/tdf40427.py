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

  def get_item(self, xTree, name):
    for i in xTree.getChildren():
        xItem = xTree.getChild(i)
        if name == get_state_as_dict(xItem)['Text']:
            return xItem

  def expand_all(self, xTreeItem):
    count = len(xTreeItem.getChildren())
    for i in xTreeItem.getChildren():
        xTreeItem.getChild(i).executeAction("EXPAND", ())
        count += self.expand_all(xTreeItem.getChild(i))
    return count

  def get_names(self, xTreeItem):
    names = []
    for i in xTreeItem.getChildren():
        names.append(get_state_as_dict(xTreeItem.getChild(str(i)))['Text'])
        names += self.get_names(xTreeItem.getChild(i))
    return names

  def test_tdf40427(self):
    with self.ui_test.load_file(get_url_for_data_file("tdf40427_SectionPositions.odt")) as document:
        xMainWindow = self.xUITest.getTopFocusWindow()
        xWriterEdit = xMainWindow.getChild("writer_edit")

        self.assertEqual(2, document.CurrentController.PageCount)

        # Make sure that the view is 2 pages side-by-side - look at dialog View-Zoom-Zoom
        with self.ui_test.execute_dialog_through_command(".uno:Zoom") as xDialog:

            columnssb = xDialog.getChild("columnssb")
            columns = xDialog.getChild("columns")
            bookmode = xDialog.getChild("bookmode")
            self.assertEqual("true", get_state_as_dict(columns)["Checked"])
            self.assertEqual("2", get_state_as_dict(columnssb)["Text"])
            self.assertEqual("false", get_state_as_dict(bookmode)["Selected"])


        # In this view, the sections "SectionB" and "SectionC" on second page are positioned on screen
        # higher than "SectionY" and "SectionA" respectively; there are nested and anchored sections.
        # Make sure that order in Navigator follows their relative position in document, not vertical
        # position on screen, nor sorted alphabetically. Sections in flying frames are sorted by their
        # anchor position in the document.

        self.xUITest.executeCommand(".uno:Sidebar")
        xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

        # wait until the navigator panel is available
        xNavigatorPanel = self.ui_test.wait_until_child_is_available('NavigatorPanel')

        xContentTree = xNavigatorPanel.getChild("contenttree")
        xSections = self.get_item(xContentTree, 'Sections')
        self.assertEqual('Sections', get_state_as_dict(xSections)['Text'])
        xSections.executeAction("EXPAND", ())
        totalSectionsCount = self.expand_all(xSections)

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
        self.assertEqual(len(refSectionNames), totalSectionsCount)

        actSectionNames = self.get_names(xSections)

        # Without the fix in place, this would fail with
        #   AssertionError: Lists differ: ['SectionZ', 'SectionY', 'SectionT3', 'SectionT1', 'SectionT2'[100 chars]onC'] != ['SectionZ', 'SectionB', 'SectionF3', 'SectionFinF3', 'Section[100 chars]onA']
        self.assertEqual(refSectionNames, actSectionNames)

        self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
