# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues
#test bookmark dialog
class bookmarkDialog(UITestCase):

    def test_bookmark_dialog(self):

        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert") as xBookDlg:
                xBmk = xBookDlg.getChild("bookmarks")
                self.assertEqual(get_state_as_dict(xBmk)["VisibleCount"], "1")  #check for 1st bookmark exist

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close") as xBookDlg:
                xBmk = xBookDlg.getChild("bookmarks")
                self.assertEqual(get_state_as_dict(xBmk)["VisibleCount"], "2")   #check for 2 bookmarks

#now delete one bookmark
            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close") as xBookDlg:
                xBmk = xBookDlg.getChild("bookmarks")
                xSecondListEntry = xBmk.getChild("1") #  select second bookmark
                xSecondListEntry.executeAction("SELECT", tuple())
                xDelBtn = xBookDlg.getChild("delete")
                xDelBtn.executeAction("CLICK", tuple())    # delete one bookmark

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close") as xBookDlg:
                xBmk2 = xBookDlg.getChild("bookmarks")
                self.assertEqual(get_state_as_dict(xBmk2)["VisibleCount"], "1")  #check for 1 bookmark


    def test_bookmark_dialog_rename(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close") as xBookDlg:
                xBmk = xBookDlg.getChild("bookmarks")
                xFirstListEntry = xBmk.getChild("0") #  select first bookmark
                xFirstListEntry.executeAction("SELECT", tuple())
                xRenameBtn = xBookDlg.getChild("rename")

                with self.ui_test.execute_blocking_action(xRenameBtn.executeAction, args=('CLICK', ())) as dialog:
                    xNewNameTxt=dialog.getChild("entry")
                    xNewNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"newname"}))

                x1stListEntry = xBmk.getChild("O") #  select first bookmark - name "newname"
                x1stListEntry.executeAction("SELECT", tuple())

                self.assertEqual(get_state_as_dict(x1stListEntry)["Text"], "1\tnewname\t\tNo\t")  #check the new name "newname"



    def test_bookmark_dialog_goto(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            type_text(xWriterEdit, "Test for bookmark")
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
            type_text(xWriterEdit, "Test2 for bookmark")

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close") as xBookDlg:
                xBmk = xBookDlg.getChild("bookmarks")
                xFirstListEntry = xBmk.getChild("0") #  select first bookmark
                xFirstListEntry.executeAction("SELECT", tuple())
                xGoToBtn = xBookDlg.getChild("goto")
                xGoToBtn.executeAction("CLICK", tuple()) # goto 1st bookmark

    def test_bookmark_dialog_edittext(self):
        with self.ui_test.create_doc_in_start_center("writer") as xDoc:

            xDoc.Text.insertString(xDoc.Text.getStart(), "foo", False)
            self.xUITest.executeCommand(".uno:SelectAll")

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="insert"):
                pass

            with self.ui_test.execute_dialog_through_command(".uno:InsertBookmark", close_button="close") as xBookDlg:
                xBmk = xBookDlg.getChild("bookmarks")
                xFirstListEntry = xBmk.getChild("0") #  select first bookmark
                xFirstListEntry.executeAction("SELECT", tuple())
                xEditBtn = xBookDlg.getChild("edittext")

                xEditBtn.executeAction('CLICK', ())

                # this does not work - the Edit widget has the focus but it's not forwarded
#                xBookDlg.executeAction("TYPE", mkPropertyValues({"TEXT":"fubar"}))
#                xBookDlg.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                # this did not work previously but now works due to explicit
                # forwarding in TreeListUIObject::execute()
                xBmk.executeAction("TYPE", mkPropertyValues({"TEXT":"fubar"}))
                xBmk.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

                x1stListEntry = xBmk.getChild("O") #  select first bookmark
                x1stListEntry.executeAction("SELECT", tuple())

                self.assertEqual(xDoc.Text.String, "fubar")
                self.assertEqual(get_state_as_dict(x1stListEntry)["Text"], "1\tBookmark 1\tfubar\tNo\t")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
