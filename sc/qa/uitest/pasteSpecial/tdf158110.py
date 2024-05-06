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
from libreoffice.calc.paste_special import reset_default_values

class tdf158110(UITestCase):
    def test_tdf158110_paste_special_multiple_cells(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            # Insert a comment in cell A1
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL":"A1"}))
            xArgs = mkPropertyValues({"Text": "Comment 1"})
            self.xUITest.executeCommandWithParameters(".uno:InsertAnnotation", xArgs)

            # Copy cell range A1:A2 to clipboard
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A2"}))
            self.xUITest.executeCommand(".uno:Copy")

            # Paste data using special options (check only comments)
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
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

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Comment 1' != ''
            # i.e., the comment was not copied
            self.assertEqual("Comment 1", get_cell_by_position(document, 0, 1, 0).Annotation.String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
