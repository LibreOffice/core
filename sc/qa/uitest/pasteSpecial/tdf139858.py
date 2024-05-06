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
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.paste_special import reset_default_values

class tdf139858(UITestCase):
    def test_tdf139858_paste_comment(self):
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

                # Without the fix in place, this test would have failed with
                # AssertionError: 'A1 sample text' != ''
                # i.e., the cell content was overwritten
                self.assertEqual(targetCell + " sample text", get_cell_by_position(document, 0, index, 0).getString())
                self.assertEqual("Comment 2", get_cell_by_position(document, 0, index, 0).Annotation.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
