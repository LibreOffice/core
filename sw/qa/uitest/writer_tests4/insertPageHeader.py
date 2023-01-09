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

class WriterInsertPageHeader(UITestCase):

    def insert_header(self):
        document = self.ui_test.get_component()

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, False)

        self.xUITest.executeCommand(
            ".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=true")

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, True)

    def delete_header(self):
        document = self.ui_test.get_component()

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, True)

        with self.ui_test.execute_dialog_through_command(
                ".uno:InsertPageHeader?PageStyle:string=Default%20Page%20Style&On:bool=false", close_button="yes"):
            pass

        self.assertEqual(
            document.StyleFamilies.PageStyles.Standard.HeaderIsOn, False)

    def test_header(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            self.insert_header()

            self.delete_header()


    def test_tdf107427(self):
        with self.ui_test.create_doc_in_start_center("writer") as document:

            self.insert_header()

            with self.ui_test.execute_dialog_through_command(".uno:InsertTable"):
                pass


            tables = document.getTextTables()
            self.assertEqual(len(tables[0].getRows()), 2)
            self.assertEqual(len(tables[0].getColumns()), 2)

            self.xUITest.executeCommand(".uno:SelectAll")

            self.delete_header()

    def test_tdf146248(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf146248.docx")):

            self.delete_header()

            # crashed before
            self.xUITest.executeCommand(".uno:Undo")

            document = self.ui_test.get_component()
            self.assertEqual(
                document.StyleFamilies.PageStyles.Standard.HeaderIsOn, True)


# vim: set shiftwidth=4 softtabstop=4 expandtab:
