# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.calc import enter_text_to_cell

#Calc sheet to Writer table

class sheetToTable(UITestCase):
    def test_sheet_to_table_without_hidden_rows(self):
        with self.ui_test.load_file(get_url_for_data_file("hiddenRow.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_file(get_url_for_data_file("tableToText.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            self.xUITest.executeCommand(".uno:Paste")
            #verify (don't copy hidden cells)
            self.assertEqual(writer_doc.TextTables.getCount(), 1)
            table = writer_doc.getTextTables()[0]
            # This was 3 (copied hidden row)
            self.assertEqual(len(table.getRows()), 2)
            self.assertEqual(table.getCellByName("A1").getString(), "1")
            # This was "2 (hidden)" (copied hidden row)
            self.assertEqual(table.getCellByName("A2").getString(), "3")

    def test_tdf138688(self):
        with self.ui_test.load_file(get_url_for_data_file("hiddenRow.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_file(get_url_for_data_file("tableToText.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            # set numbering in the paragraph after the table
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:GoDown")
            self.xUITest.executeCommand(".uno:DefaultNumbering")
            self.xUITest.executeCommand(".uno:GoUp")
            self.xUITest.executeCommand(".uno:GoUp")

            #verify (this was a freezing/crash)
            self.xUITest.executeCommand(".uno:Paste")

            #verify also tdf#124646 (don't copy hidden cells)
            self.assertEqual(writer_doc.TextTables.getCount(), 1)
            table = writer_doc.getTextTables()[0]
            # This was 3 (copied hidden row)
            self.assertEqual(len(table.getRows()), 2)
            self.assertEqual(table.getCellByName("A1").getString(), "1")
            # This was "2 (hidden)" (copied hidden row)
            self.assertEqual(table.getCellByName("A2").getString(), "3")

    def test_tdf129083(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Test 1")
            enter_text_to_cell(gridwin, "A2", "Test 2")
            enter_text_to_cell(gridwin, "A3", "Test 3")
            enter_text_to_cell(gridwin, "A4", "Test 4")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            self.xUITest.executeCommand(".uno:Copy")


        with self.ui_test.load_empty_file("writer") as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:InsertTable?Columns:short=1&Rows:short=4")

            self.xUITest.executeCommand(".uno:Paste")

            self.assertEqual(writer_doc.TextTables.getCount(), 1)
            table = writer_doc.getTextTables()[0]
            self.assertEqual(len(table.getRows()), 4)
            self.assertEqual(table.getCellByName("A1").getString(), "Test 1")
            self.assertEqual(table.getCellByName("A2").getString(), "Test 2")
            self.assertEqual(table.getCellByName("A3").getString(), "Test 3")
            self.assertEqual(table.getCellByName("A4").getString(), "Test 4")

    def test_tdf116685(self):
        with self.ui_test.create_doc_in_start_center("calc"):

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "Test 1")
            enter_text_to_cell(gridwin, "A2", "Test 2")
            enter_text_to_cell(gridwin, "A3", "Test 3")
            enter_text_to_cell(gridwin, "A4", "Test 4")

            self.xUITest.executeCommand(".uno:SelectAll")
            self.xUITest.executeCommand(".uno:Copy")

        with self.ui_test.load_empty_file("writer") as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()

            self.xUITest.executeCommand(".uno:Paste")

            # Without the fix in place, this test would have failed with
            # AssertionError: 0 != 1
            self.assertEqual(writer_doc.TextTables.getCount(), 1)
            table = writer_doc.getTextTables()[0]
            self.assertEqual(len(table.getRows()), 4)
            self.assertEqual(table.getCellByName("A1").getString(), "Test 1")
            self.assertEqual(table.getCellByName("A2").getString(), "Test 2")
            self.assertEqual(table.getCellByName("A3").getString(), "Test 3")
            self.assertEqual(table.getCellByName("A4").getString(), "Test 4")

# vim: set shiftwidth=4 softtabstop=4 expandtab: