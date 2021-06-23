# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class WriterInsertPageFooter(UITestCase):

    def insert_footer(self):
        document = self.ui_test.get_component()

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.FooterIsOn, False)

        self.xUITest.executeCommand(
            ".uno:InsertPageFooter?PageStyle:string=Default%20Page%20Style&On:bool=true")

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.FooterIsOn, True)

    def delete_footer(self):
        document = self.ui_test.get_component()

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.FooterIsOn, True)

        self.ui_test.execute_dialog_through_command(
            ".uno:InsertPageFooter?PageStyle:string=Default%20Page%20Style&On:bool=false")

        xDialog = self.xUITest.getTopFocusWindow()

        xOption = xDialog.getChild("yes")
        xOption.executeAction("CLICK", tuple())

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.FooterIsOn, False)

    def test_footer(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.insert_footer()

        self.delete_footer()

        self.ui_test.close_doc()

    def test_tdf107427(self):
        self.ui_test.create_doc_in_start_center("writer")

        self.insert_footer()

        self.ui_test.execute_dialog_through_command(".uno:InsertTable")

        xInsertDlg = self.xUITest.getTopFocusWindow()

        xOkBtn = xInsertDlg.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())

        document = self.ui_test.get_component()

        tables = document.getTextTables()
        self.assertEqual(len(tables[0].getRows()), 2)
        self.assertEqual(len(tables[0].getColumns()), 2)

        self.xUITest.executeCommand(".uno:SelectAll")

        self.delete_footer()

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
