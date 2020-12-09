# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
import org.libreoffice.unotest
import pathlib

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#Calc sheet to Writer table

class sheetToTable(UITestCase):
    def test_sheet_to_table_without_hidden_rows(self):
        print(get_url_for_data_file("hiddenRow.ods"))
        calc_doc = self.ui_test.load_file(get_url_for_data_file("hiddenRow.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        self.ui_test.close_doc()
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tableToText.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        self.xUITest.executeCommand(".uno:Paste")
        #verify (don't copy hidden cells)
        self.assertEqual(document.TextTables.getCount(), 1)
        table = document.getTextTables()[0]
        # This was 3 (copied hidden row)
        self.assertEqual(len(table.getRows()), 2)
        self.assertEqual(table.getCellByName("A1").getString(), "1")
        # This was "2 (hidden)" (copied hidden row)
        self.assertEqual(table.getCellByName("A2").getString(), "3")
        self.ui_test.close_doc()

    def test_tdf138688(self):
        print(get_url_for_data_file("hiddenRow.ods"))
        calc_doc = self.ui_test.load_file(get_url_for_data_file("hiddenRow.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        self.xUITest.executeCommand(".uno:SelectAll")
        self.xUITest.executeCommand(".uno:Copy")
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        self.ui_test.close_doc()
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tableToText.odt"))
        document = self.ui_test.get_component()
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
        self.assertEqual(document.TextTables.getCount(), 1)
        table = document.getTextTables()[0]
        # This was 3 (copied hidden row)
        self.assertEqual(len(table.getRows()), 2)
        self.assertEqual(table.getCellByName("A1").getString(), "1")
        # This was "2 (hidden)" (copied hidden row)
        self.assertEqual(table.getCellByName("A2").getString(), "3")
        self.ui_test.close_doc()
