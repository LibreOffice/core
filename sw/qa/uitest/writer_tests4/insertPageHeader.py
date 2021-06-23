#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class WriterInsertPageHeader(UITestCase):

    def insert_header(self, document):
        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, False)

        self.xUITest.executeCommand(
            ".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=true")

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, True)

    def delete_header(self, document):
        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, True)

        self.ui_test.execute_dialog_through_command(
            ".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=false")

        xDialog = self.xUITest.getTopFocusWindow()

        xOption = xDialog.getChild("yes")
        xOption.executeAction("CLICK", tuple())

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, False)

    def test_header(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.insert_header(document)

            self.delete_header(document)

    def test_tdf107427(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.insert_header(document)

            self.ui_test.execute_dialog_through_command(".uno:InsertTable")

            xInsertDlg = self.xUITest.getTopFocusWindow()

            xOkBtn = xInsertDlg.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())


            tables = document.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 2)
            self.assertEqual(len(tables[0].getColumns()), 2)

            self.xUITest.executeCommand(".uno:SelectAll")

            self.delete_header(document)

