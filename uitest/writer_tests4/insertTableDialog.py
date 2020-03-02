#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class WriterInsertTableDialog(UITestCase):

    def insert_table(self, name, rows, cols):

        self.ui_test.create_doc_in_start_center("writer")

        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()

        xNameEdit = xDialog.getChild("nameedit")

        xNameEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xNameEdit.executeAction("TYPE", mkPropertyValues({"TEXT": name}))

        xColSpin = xDialog.getChild("colspin")
        xColSpin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xColSpin.executeAction("TYPE", mkPropertyValues({"TEXT": str(cols)}))

        xRowSpin = xDialog.getChild("rowspin")
        xRowSpin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xRowSpin.executeAction("TYPE", mkPropertyValues({"TEXT": str(rows)}))

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        tables = document.getTextTables()

        self.assertEqual(tables[0].getName(), name)
        self.assertEqual(len(tables[0].getRows()), rows)
        self.assertEqual(len(tables[0].getColumns()), cols)

    def insertTextIntoCell(self, table, cellName, text ):
        tableText = table.getCellByName( cellName )
        tableText.setString( text )

    def test_tdf80663(self):

        self.insert_table("Test1", 2, 2)

        document = self.ui_test.get_component()

        tables = document.getTextTables()
        self.xUITest.executeCommand(".uno:DeleteRows")

        self.assertEqual(len(tables[0].getRows()), 1)
        self.assertEqual(len(tables[0].getColumns()), 2)

        self.xUITest.executeCommand(".uno:Undo")

        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[0].getColumns()), 2)

        self.ui_test.close_doc()

    def test_tdf96067(self):

        self.insert_table("Test2", 3, 3)

        self.xUITest.executeCommand(".uno:SelectTable")
        self.xUITest.executeCommand(".uno:InsertRowsBefore")

        document = self.ui_test.get_component()
        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 6)
        self.assertEqual(len(tables[0].getColumns()), 3)

        self.xUITest.executeCommand(".uno:Undo")

        self.ui_test.close_doc()

    def test_tdf104158(self):

        self.insert_table("Test3", 2, 2)

        self.ui_test.execute_dialog_through_command(".uno:TableNumberFormatDialog")

        xNumberFormatDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xNumberFormatDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_tdf87199(self):

        self.insert_table("Test4", 2, 1)

        document = self.ui_test.get_component()
        tables = document.getTextTables()
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

    def test_cancel_button_insert_line_break_dialog(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        Dialog = self.xUITest.getTopFocusWindow()
        CancelBtn = Dialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(CancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
