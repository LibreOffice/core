# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.paste_special import reset_default_values
from uitest.uihelper.common import get_state_as_dict

class tdf160765(UITestCase):
    def test_tdf160765_paste_special_comments_checked(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            # Insert a comment in cell A1
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL":"A1"}))
            xArgs = mkPropertyValues({"Text": "Comment 1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # Copy cell A1 to clipboard
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:Copy")

            # Paste data using special options (check only comments)
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xPasteSpecialDlg:
                reset_default_values(self, xPasteSpecialDlg)
                xDateTimeChkBox = xPasteSpecialDlg.getChild("datetime")
                xDateTimeChkBox.executeAction("CLICK", tuple())
                xTextChkBox = xPasteSpecialDlg.getChild("text")
                xTextChkBox.executeAction("CLICK", tuple())
                xNumbersChkBox = xPasteSpecialDlg.getChild("numbers")
                xNumbersChkBox.executeAction("CLICK", tuple())
                xCommentsChkBox = xPasteSpecialDlg.getChild("comments")
                xCommentsChkBox.executeAction("CLICK", tuple())

            # After tdf#158110 when an existing comment is overwritten, a confirmation dialog is shown, so close dialog without any action
            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial", close_button="cancel") as xPasteSpecialDlg:
                xCommentsChkBox = xPasteSpecialDlg.getChild("comments")
                # Without the fix in place, this test would have failed with
                # AssertionError: 'true' != 'false'
                # i.e., the comments checkbox was not remembered
                self.assertEqual(get_state_as_dict(xCommentsChkBox)["Selected"], "true")

    def test_tdf160765_undo_paste_comment(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            # Write text to cell A1 and B1
            enter_text_to_cell(xGridWin, "A1", "A1 sample text")
            enter_text_to_cell(xGridWin, "B1", "B1 sample text")

            # Insert a comment in cell B1
            xArgs = mkPropertyValues({"Text": "Comment 1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # Insert a comment in cell A2
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL":"A2"}))
            xArgs = mkPropertyValues({"Text": "Comment 2"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # Copy cell A2 to clipboard
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            self.xUITest.executeCommand(".uno:Copy")

            # Set cursor to cells and paste data using special options (check only comments)
            targetCells = ["A1", "B1"]
            for index, targetCell in enumerate(targetCells):
                xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": targetCell}))
                with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xPasteSpecialDlg:
                    reset_default_values(self, xPasteSpecialDlg)
                    xDateTimeChkBox = xPasteSpecialDlg.getChild("datetime")
                    xDateTimeChkBox.executeAction("CLICK", tuple())
                    xTextChkBox = xPasteSpecialDlg.getChild("text")
                    xTextChkBox.executeAction("CLICK", tuple())
                    xNumbersChkBox = xPasteSpecialDlg.getChild("numbers")
                    xNumbersChkBox.executeAction("CLICK", tuple())
                    xCommentsChkBox = xPasteSpecialDlg.getChild("comments")
                    xCommentsChkBox.executeAction("CLICK", tuple())

                # After tdf#158110 when an existing comment is overwritten, a confirmation dialog is shown
                xCheckWarningDlg = self.xUITest.getTopFocusWindow()
                if get_state_as_dict(xCheckWarningDlg)["ID"] == "CheckWarningDialog":
                    xYesBtn = xCheckWarningDlg.getChild("yes")
                    xYesBtn.executeAction("CLICK", tuple())

            # Undo both inserted comments
            self.xUITest.executeCommand(".uno:Undo")
            # Without the fix in place, this test would have failed with
            # AssertionError: 'Comment 1' != ''
            # i.e., the cell does not contain any comment
            self.assertEqual("Comment 1", get_cell_by_position(document, 0, 1, 0).Annotation.String)
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual("", get_cell_by_position(document, 0, 0, 0).Annotation.String)

            # Redo both inserted comments
            self.xUITest.executeCommand(".uno:Redo")
            # Without the fix in place, this test would have failed with
            # AssertionError: 'Comment 2' != ''
            # i.e., the cell does not contain the restored comment
            self.assertEqual("Comment 2", get_cell_by_position(document, 0, 0, 0).Annotation.String)
            self.xUITest.executeCommand(".uno:Redo")
            # Without the fix in place, this test would have failed with
            # AssertionError: 'Comment 2' != ''
            # i.e., the cell does not contain the restored comment
            self.assertEqual("Comment 2", get_cell_by_position(document, 0, 0, 0).Annotation.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
