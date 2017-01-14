#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf96067(UITestCase):

    def test_insert_row_and_undo(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertTable")

        xInsertDlg = self.xUITest.getTopFocusWindow()

        xRowSpin = xInsertDlg.getChild("rowspin")
        xRowSpin.executeAction("UP", tuple())

        xColSpin = xInsertDlg.getChild("colspin")
        xColSpin.executeAction("UP", tuple())

        xOkBtn = xInsertDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 3)
        self.assertEqual(len(tables[0].getColumns()), 3)

        self.xUITest.executeCommand(".uno:SelectTable")
        self.xUITest.executeCommand(".uno:InsertRowsBefore")

        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 6)
        self.assertEqual(len(tables[0].getColumns()), 3)

        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
