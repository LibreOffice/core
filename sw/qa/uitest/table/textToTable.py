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
from uitest.uihelper.common import get_url_for_data_file, type_text

#Writer Text to table

class textToTable(UITestCase):
    def test_text_to_table(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #Enter A;B ; select the text ; dialog Text to table - Semicolon; verify
            type_text(xWriterEdit, "A;B;C")
            xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "5"}))
            with self.ui_test.execute_dialog_through_command(".uno:ConvertTextToTable") as xDialog:
                semicolons = xDialog.getChild("semicolons")
                semicolons.executeAction("CLICK", tuple())
            #verify
            self.assertEqual(document.TextTables.getCount(), 1)
            tables = document.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 1)
            self.assertEqual(len(tables[0].getColumns()), 3)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(document.TextTables.getCount(), 0)
            self.assertEqual(document.Text.String[0:5], "A;B;C")


    def test_text_to_table_header(self):
        with self.ui_test.load_file(get_url_for_data_file("textToTable.odt")) as writer_doc:
            xWriterDoc = self.xUITest.getTopFocusWindow()
            xWriterEdit = xWriterDoc.getChild("writer_edit")
            #open file; select all text ; dialog Text to table - other ":"; verify
            self.xUITest.executeCommand(".uno:SelectAll")
            with self.ui_test.execute_dialog_through_command(".uno:ConvertTextToTable") as xDialog:
                other = xDialog.getChild("other")
                other.executeAction("CLICK", tuple())
                othered = xDialog.getChild("othered")
                othered.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                othered.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                othered.executeAction("TYPE", mkPropertyValues({"TEXT":":"}))
                headingcb = xDialog.getChild("headingcb")
                headingcb.executeAction("CLICK", tuple())
            #verify
            self.assertEqual(writer_doc.TextTables.getCount(), 1)
            tables = writer_doc.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 4)
            self.assertEqual(len(tables[0].getColumns()), 3)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(writer_doc.TextTables.getCount(), 0)
            self.assertEqual(writer_doc.Text.String[0:5], "A:B:C")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
