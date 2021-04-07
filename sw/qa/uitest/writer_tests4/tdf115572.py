# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class tdf115572(UITestCase):

   def insertTextIntoCell(self, table, cellName, text ):
        tableText = table.getCellByName( cellName )
        tableText.setString( text )

   def test_tdf115572_table_format_gets_reset_deleting_row(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #insert table 2x2
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)
        #select font format - Bold and write text "abc"
        self.xUITest.executeCommand(".uno:Bold")
        tables = document.getTextTables()
        self.insertTextIntoCell(tables[0], "A1", "abc" )
        #go to second row - arrow down
        self.xUITest.executeCommand(".uno:GoDown")
        #delete row
        self.xUITest.executeCommand(".uno:DeleteRows")
        #now the cursor is in the second column - go left
        self.xUITest.executeCommand(".uno:GoLeft")
        self.xUITest.executeCommand(".uno:GoLeft")
        #check the format of the text (should be still bold) Format-Character-Font-Style-Bold
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xweststylelbcjk = xDialog.getChild("weststylelb-cjk")
        self.assertEqual(get_state_as_dict(xweststylelbcjk)["Text"], "Bold")
        xCloseBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()

   def test_tdf115572_table_format_gets_reset_deleting_column(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #insert table 2x2
        self.ui_test.execute_dialog_through_command(".uno:InsertTable")
        xDialog = self.xUITest.getTopFocusWindow()
        xOkBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOkBtn)
        #select font format - Bold and write text "abc"
        self.xUITest.executeCommand(".uno:Bold")
        tables = document.getTextTables()
        self.insertTextIntoCell(tables[0], "A1", "abc" )
        #go to second column
        self.xUITest.executeCommand(".uno:GoRight")
        #delete column
        self.xUITest.executeCommand(".uno:DeleteColumns")
        #check the format of the text (should be still bold) Format-Character-Font-Style-Bold
        self.ui_test.execute_dialog_through_command(".uno:FontDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xweststylelbcjk = xDialog.getChild("weststylelb-cjk")
        self.assertEqual(get_state_as_dict(xweststylelbcjk)["Text"], "Bold")
        xCloseBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCloseBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
