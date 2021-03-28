# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

# Bug 59121 - Redo/Undo loses bookmark: https://bugs.documentfoundation.org/show_bug.cgi?id=59121

class tdf59121(UITestCase):
    def test_tdf59121_bookmark_is_lost(self):
        MainDoc = self.ui_test.create_doc_in_start_center("writer")
        MainWindow = self.xUITest.getTopFocusWindow()
        self.xUITest.executeCommand(".uno:UpdateInputFields")
        writer_edit = MainWindow.getChild("writer_edit")

        # add some text to the document
        writer_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "h"}))
        writer_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))
        writer_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "l"}))
        writer_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "l"}))
        writer_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "o"}))

        # add a bookmark named 'foo'
        writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "1", "START_POS": "4"}))
        self.xUITest.executeCommand(".uno:InsertBookmark")
        InsertBookmarkDialog = self.xUITest.getTopFocusWindow()
        name = InsertBookmarkDialog.getChild("name")
        name.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+a"}))
        name.executeAction("SELECT", mkPropertyValues({"FROM": "0", "TO": "10"}))
        name.executeAction("TYPE", mkPropertyValues({"TEXT": "f"}))
        name.executeAction("TYPE", mkPropertyValues({"TEXT": "o"}))
        name.executeAction("TYPE", mkPropertyValues({"TEXT": "o"}))
        insert = InsertBookmarkDialog.getChild("insert")
        self.ui_test.close_dialog_through_button(insert)

        # select the whole document
        writer_edit = MainWindow.getChild("writer_edit")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+a"}))
        self.xUITest.executeCommand(".uno:SelectAll")
        writer_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "5"}))

        # Cut all text then undo the cut.  Repeat multiple times.
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+x"}))
        self.xUITest.executeCommand(".uno:Cut")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+z"}))
        self.xUITest.executeCommand(".uno:Undo")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+x"}))
        self.xUITest.executeCommand(".uno:Cut")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+z"}))
        self.xUITest.executeCommand(".uno:Undo")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+x"}))
        self.xUITest.executeCommand(".uno:Cut")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+z"}))
        self.xUITest.executeCommand(".uno:Undo")

        # redo and undo
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+y"}))
        self.xUITest.executeCommand(".uno:Redo")
        writer_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+z"}))
        self.xUITest.executeCommand(".uno:Undo")

        # assert that one bookmark exists and then assert that the bookmark is named "foo"
        bookmarks = self.ui_test.get_component().Bookmarks
        self.assertEqual(1, len(bookmarks))
        self.assertEqual("foo", bookmarks[0].Name)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
