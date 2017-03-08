#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf87199(UITestCase):

    def insertTextIntoCell(self, table, cellName, text ):
        tableText = table.getCellByName( cellName )
        tableText.setString( text )

    def test_merge_column(self):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertTable")

        xInsertDlg = self.xUITest.getTopFocusWindow()


        xColSpin = xInsertDlg.getChild("colspin")
        xColSpin.executeAction("DOWN", tuple())

        xOkBtn = xInsertDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[0].getColumns()), 1)

        self.insertTextIntoCell(tables[0], "A1", "test" )
        self.insertTextIntoCell(tables[0], "A2", "test" )

        cursor = tables[0].getCellByName( "A1" ).createTextCursor()

        self.xUITest.executeCommand(".uno:EntireColumn")

        self.xUITest.executeCommand(".uno:MergeCells")

        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 1)
        self.assertEqual(len(tables[0].getColumns()), 1)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[0].getColumns()), 1)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
