# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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

        with self.ui_test.execute_dialog_through_command(
                ".uno:InsertPageFooter?PageStyle:string=Default%20Page%20Style&On:bool=false", close_button="yes"):
            pass

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.FooterIsOn, False)

    def test_footer(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.insert_footer()

            self.delete_footer()


    def test_tdf107427(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.insert_footer()

            with self.ui_test.execute_dialog_through_command(".uno:InsertTable"):
                pass


            tables = document.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 2)
            self.assertEqual(len(tables[0].getColumns()), 2)

            self.xUITest.executeCommand(".uno:SelectAll")

            self.delete_footer()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
