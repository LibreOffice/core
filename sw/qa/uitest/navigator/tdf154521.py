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
from uitest.uihelper.common import get_state_as_dict
from com.sun.star.view import XSelectionChangeListener
import unohelper

selectionChangedResult = 0

class XSelectionChangeListenerExtended(unohelper.Base, XSelectionChangeListener):
    # is invoked when a text selected, e.g. by selecting a bookmark in the Navigator
    @classmethod
    def selectionChanged(self, event):
        global selectionChangedResult
        selection = event.Source.getSelection()
        if selection.supportsService("com.sun.star.text.TextRanges"):
            textRange = selection.getByIndex(0)
            if textRange.getString() == "foo":
                selectionChangedResult = 1
            else:
                selectionChangedResult = -1

    @classmethod
    def disposing(self, event):
        pass

class tdf154521(UITestCase):

    def test_tdf154521(self):
        global selectionChangedResult
        with self.ui_test.create_doc_in_start_center("writer") as xDoc:

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("content5")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'root' button

            # type "foo", and create a bookmark on it

            self.xUITest.executeCommand(".uno:Escape")

            xDoc.Text.insertString(xDoc.Text.getStart(), "foo", False)
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            xWriterEdit.executeAction("FOCUS", tuple())

            # add XSelectionChangeListener

            xListener = XSelectionChangeListenerExtended()
            self.assertIsNotNone(xListener)
            xDoc.CurrentController.addSelectionChangeListener(xListener)

            xContentTree = xNavigatorPanel.getChild("contenttree")

            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            # select bookmark to trigger selectionChanged event

            xContentTree.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            # wait for handling the selectionChanged event
            while selectionChangedResult == 0:
                pass

            # This was -1 (missing selection because of early broadcasting)
            self.assertEqual(selectionChangedResult, 1)

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
