# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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
        with self.ui_test.load_file(get_url_for_data_file("splitTable.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #go to row 2
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            #dialog Split table, check Copy heading, OK -> verify 2 tables, 1st has 2 rows, second has 5 rows
            with self.ui_test.execute_dialog_through_command(".uno:SplitTable") as xDialog:

                copyheading = xDialog.getChild("copyheading")
                copyheading.executeAction("CLICK", tuple())
            self.assertEqual(writer_doc.TextTables.getCount(), 2)
            tables = writer_doc.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 2)
            self.assertEqual(len(tables[1].getRows()), 5)
            #undo -> verify 1 tables
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

        #dialog Split table, check Custom heading, OK -> verify 2 tables, 1st has 2 rows, second has 4 rows
        with self.ui_test.load_file(get_url_for_data_file("splitTable.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #go to row 2
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            with self.ui_test.execute_dialog_through_command(".uno:SplitTable") as xDialog:

                customheading = xDialog.getChild("customheading")
                customheading.executeAction("CLICK", tuple())
            self.assertEqual(writer_doc.TextTables.getCount(), 2)
            tables = writer_doc.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 2)
            self.assertEqual(len(tables[1].getRows()), 4)
            #undo -> verify 1 tables
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

        #dialog Split table, check No heading, OK -> verify 2 tables, 1st has 2 rows, second has 4 rows
        with self.ui_test.load_file(get_url_for_data_file("splitTable.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            #go to row 2
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            with self.ui_test.execute_dialog_through_command(".uno:SplitTable") as xDialog:

                noheading = xDialog.getChild("noheading")
                noheading.executeAction("CLICK", tuple())
            self.assertEqual(writer_doc.TextTables.getCount(), 2)
            tables = writer_doc.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 2)
            self.assertEqual(len(tables[1].getRows()), 4)
            #undo -> verify 1 tables
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 1)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
