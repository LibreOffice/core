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

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xInsertBtn = xBookDlg.getChild("insert")
        xInsertBtn.executeAction("CLICK", tuple()) # first bookmark

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xInsertBtn = xBookDlg.getChild("insert")
        xBmk = xBookDlg.getChild("bookmarks")
        self.assertEqual(get_state_as_dict(xBmk)["VisibleCount"], "1")  #check for 1st bookmark exist
        xInsertBtn.executeAction("CLICK", tuple())    # add 2nd bookmark

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xCloseBtn = xBookDlg.getChild("close")
        xCloseBtn.executeAction("CLICK", tuple())    # close

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xBmk = xBookDlg.getChild("bookmarks")
        self.assertEqual(get_state_as_dict(xBmk)["VisibleCount"], "2")   #check for 2 bookmarks
        xCloseBtn = xBookDlg.getChild("close")
        xCloseBtn.executeAction("CLICK", tuple())    # close dialog

#now delete one bookmark
        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xBmk = xBookDlg.getChild("bookmarks")
        xSecondListEntry = xBmk.getChild("1") #  select second bookmark
        xSecondListEntry.executeAction("SELECT", tuple())
        xDelBtn = xBookDlg.getChild("delete")
        xDelBtn.executeAction("CLICK", tuple())    # delete one bookmark
        xCloseBtn = xBookDlg.getChild("close")
        xCloseBtn.executeAction("CLICK", tuple())    # close dialog

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xCloseBtn = xBookDlg.getChild("close")
        xBmk2 = xBookDlg.getChild("bookmarks")
        self.assertEqual(get_state_as_dict(xBmk2)["VisibleCount"], "1")  #check for 1 bookmark
        xCloseBtn.executeAction("CLICK", tuple())    # now we have only 1 bookmark

        self.ui_test.close_doc()



    def test_bookmark_dialog_rename(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xInsertBtn = xBookDlg.getChild("insert")
        xInsertBtn.executeAction("CLICK", tuple()) # first bookmark

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xBmk = xBookDlg.getChild("bookmarks")
        xFirstListEntry = xBmk.getChild("0") #  select first bookmark
        xFirstListEntry.executeAction("SELECT", tuple())
        xRenameBtn = xBookDlg.getChild("rename")

        def handle_rename_dlg(dialog):                     #handle rename dialog - need special handling

            xNewNameTxt=dialog.getChild("entry")
            xNewNameTxt.executeAction("TYPE", mkPropertyValues({"TEXT":"newname"}))
            xOKBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_blocking_action(xRenameBtn.executeAction, args=('CLICK', ()),
                dialog_handler=handle_rename_dlg)             #close rename dialog with OK button

        xBookDlg = self.xUITest.getTopFocusWindow()
        x1stListEntry = xBmk.getChild("O") #  select first bookmark - name "newname"
        x1stListEntry.executeAction("SELECT", tuple())

        self.assertEqual(get_state_as_dict(x1stListEntry)["Text"], "1\tnewname\t\tNo\t")  #check the new name "newname"

        xCloseBtn = xBookDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

    def test_bookmark_dialog_goto(self):
        self.ui_test.create_doc_in_start_center("writer")
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xInsertBtn = xBookDlg.getChild("insert")
        xInsertBtn.executeAction("CLICK", tuple()) # first bookmark


        type_text(xWriterEdit, "Test for bookmark")
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        type_text(xWriterEdit, "Test2 for bookmark")


        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xInsertBtn = xBookDlg.getChild("insert")
        xInsertBtn.executeAction("CLICK", tuple()) # second bookmark

        self.ui_test.execute_dialog_through_command(".uno:InsertBookmark")
        xBookDlg = self.xUITest.getTopFocusWindow()
        xBmk = xBookDlg.getChild("bookmarks")
        xFirstListEntry = xBmk.getChild("0") #  select first bookmark
        xFirstListEntry.executeAction("SELECT", tuple())
        xGoToBtn = xBookDlg.getChild("goto")
        xGoToBtn.executeAction("CLICK", tuple()) # goto 1st bookmark
        xCloseBtn = xBookDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCloseBtn)


        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
