# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file

#Writer Split Table

class splitTable(UITestCase):
    def test_split_table(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("splitTable.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #go to row 2
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        #dialog Split table, check Copy heading, OK -> verify 2 tables, 1st has 2 rows, second has 5 rows
        self.ui_test.execute_dialog_through_command(".uno:SplitTable")
        xDialog = self.xUITest.getTopFocusWindow()

        copyheading = xDialog.getChild("copyheading")
        copyheading.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(document.TextTables.getCount(), 2)
        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[1].getRows()), 5)
        #undo -> verify 1 tables
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()

        #dialog Split table, check Custom heading, OK -> verify 2 tables, 1st has 2 rows, second has 4 rows
        writer_doc = self.ui_test.load_file(get_url_for_data_file("splitTable.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #go to row 2
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.ui_test.execute_dialog_through_command(".uno:SplitTable")
        xDialog = self.xUITest.getTopFocusWindow()

        customheading = xDialog.getChild("customheading")
        customheading.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(document.TextTables.getCount(), 2)
        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[1].getRows()), 4)
        #undo -> verify 1 tables
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()
        #dialog Split table, check No heading, OK -> verify 2 tables, 1st has 2 rows, second has 4 rows
        writer_doc = self.ui_test.load_file(get_url_for_data_file("splitTable.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        #go to row 2
        self.xUITest.executeCommand(".uno:GoDown")
        self.xUITest.executeCommand(".uno:GoDown")
        self.ui_test.execute_dialog_through_command(".uno:SplitTable")
        xDialog = self.xUITest.getTopFocusWindow()

        noheading = xDialog.getChild("noheading")
        noheading.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(document.TextTables.getCount(), 2)
        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[1].getRows()), 4)
        #undo -> verify 1 tables
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(document.TextTables.getCount(), 1)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
