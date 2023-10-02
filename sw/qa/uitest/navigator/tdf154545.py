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

class tdf154545(UITestCase):

    def test_tdf154545(self):
        global selectionChangedResult
        with self.ui_test.create_doc_in_start_center("writer") as xDoc:

            # click on the bookmark name in the Navigator

            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            self.xUITest.executeCommand(".uno:Sidebar")
            xWriterEdit.executeAction("SIDEBAR", mkPropertyValues({"PANEL": "SwNavigatorPanel"}))

            xNavigatorPanel = xWriterEdit.getChild("NavigatorPanel")
            xToolBar = xNavigatorPanel.getChild("content5")
            xToolBar.executeAction("CLICK", mkPropertyValues({"POS": "0"})) # 'root' button

            # type "foo", and create a bookmark on it

            xDoc.Text.insertString(xDoc.Text.getStart(), "foo", False)
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            # check selected bookmark in Navigator

            xWriterEdit.executeAction("FOCUS", tuple())

            xContentTree = xNavigatorPanel.getChild("contenttree")

            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            self.xUITest.executeCommand(".uno:Escape")

            # create a nested bookmark on the last "o"

            cursor = xDoc.getCurrentController().getViewCursor()
            cursor.goLeft(1, True)

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            self.xUITest.executeCommand(".uno:Escape")

            # check selected nested bookmark in Navigator

            # This never occurred: Navigator didn't track nested bookmarks
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmark 2")
            # This was Bookmark 1
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmark 2")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            # Select nested bookmark in Navigator

            xContentTree.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

            # This jumped to Bookmark 1 after selection
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmark 2")
            # This was Bookmark 1
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmark 2")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            # Try the same selection with Bookmark 1
            xContentTree.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")
            xContentTree.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmark 1")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectionCount"], "1")

            # go to the previous item
            xContentTree.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
            self.ui_test.wait_until_property_is_updated(xContentTree, "SelectEntryText", "Bookmarks")
            self.assertEqual(get_state_as_dict(xContentTree)["SelectEntryText"], "Bookmarks")

            self.xUITest.executeCommand(".uno:Sidebar")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
