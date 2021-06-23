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


class ManualCalcTests(UITestCase):
    def test_paste_special(self):
        # EN-8:Paste special with options
        # This test is to check that paste special combined with some options and link is ok.
        # Refers to tdf#84810

        self.ui_test.create_doc_in_start_center("calc")

        # Write text to cell A1
        xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
        enter_text_to_cell(xGridWin, "A1", "abcd")

        # Copy cell A1 to clipboard
        xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:Copy")

        # Set cursor to cell A3
        xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))

        # Choose Paste Special Options and paste data
        self.ui_test.execute_dialog_through_command(".uno:PasteSpecial")
        xPasteSpecialDlg = self.xUITest.getTopFocusWindow()
        xAllChkBox = xPasteSpecialDlg.getChild("paste_all")
        xAllChkBox.executeAction("CLICK", tuple())
        xLinkChkBox = xPasteSpecialDlg.getChild("link")
        xLinkChkBox.executeAction("CLICK", tuple())
        xOkBtn = xPasteSpecialDlg.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)

        # Assert successful paste
        document = self.ui_test.get_component()
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "abcd")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
