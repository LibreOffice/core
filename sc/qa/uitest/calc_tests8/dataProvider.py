# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from org.libreoffice.unotest import systemPathToFileUrl
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from tempfile import TemporaryDirectory

import os

class DataProvider(UITestCase):

    def test_brokenCSV(self):

        with self.ui_test.create_doc_in_start_center("calc"):

            # tdf#168497: without the fix in place, it would have crashed here
            with self.ui_test.execute_dialog_through_command(".uno:DataProvider") as xDialog:
                xBrowse = xDialog.getChild("browse")
                xApply = xDialog.getChild("apply")
                xProvider = xDialog.getChild("provider_lst")
                xURL = xDialog.getChild("ed_url")

                with self.ui_test.execute_blocking_action(
                        xBrowse.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    xFileName = dialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf165658.csv")}))
                    xOpen = dialog.getChild("open")
                    xOpen.executeAction("CLICK", tuple())

                self.assertEqual(get_url_for_data_file("tdf165658.csv"), get_state_as_dict(xURL)['Text'])
                self.assertEqual("CSV", get_state_as_dict(xProvider)['DisplayText'])

                # tdf#165658: without the fix in place, it would have crashed here
                # The fix for error 169049 changes the process slightly. The problem of no database
                # range being specified is now detected earlier and an error message is displayed to
                # the user.
                with self.ui_test.execute_blocking_action(
                        xApply.executeAction, args=('CLICK', ()), close_button="close") as dialog:
                    pass

    def do_import(self, data_format, test_file, identifier, expected_value):
        with self.ui_test.create_doc_in_start_center("calc") as xDoc:

            # Make a database range on A1:K11 called "TestDB"
            with self.ui_test.execute_dialog_through_command(".uno:DefineDBName") as xDialog:
                xName = xDialog.getChild("entry")
                xName.executeAction("SET", mkPropertyValues({"TEXT": "TestDB"}))

                xRange = xDialog.getChild("assign")
                xRange.executeAction("SET", mkPropertyValues({"TEXT": "$Sheet1.$A$1:$K$11"}))

            # Set the provider for the range to be the data from the file
            with self.ui_test.execute_dialog_through_command(".uno:DataProvider") as xDialog:
                xRange = xDialog.getChild("select_db_range")
                select_by_text(xRange, "TestDB")

                xFormat = xDialog.getChild("provider_lst")
                select_by_text(xFormat, data_format)

                xURL = xDialog.getChild("ed_url")
                xURL.executeAction("SET", mkPropertyValues({"TEXT": test_file}))

                xId = xDialog.getChild("ed_id")
                xId.executeAction("SET", mkPropertyValues({"TEXT": identifier}))

            # Check that the import updated the A1 cell.
            self.assertEqual(get_cell_by_position(xDoc, 0, 0, 0).getString(), expected_value)

    def test_html_import(self):
        # tdf#169077: Without the fix the none of the data gets imported
        test_file = os.path.join(os.getenv("SRCDIR"),
                                 "sc", "qa", "unit", "data", "dataprovider", "html",
                                 "test1.html")
        self.do_import("HTML", test_file, "//table", "Col1")

    def test_csv_import(self):
        test_file = os.path.join(os.getenv("SRCDIR"),
                                 "sc", "qa", "unit", "data", "dataprovider", "csv",
                                 "test1.csv")
        self.do_import("CSV", test_file, "", "1")

    def test_save_split_transformation(self):

        with TemporaryDirectory() as tempdir:

            xFilePath = os.path.join(tempdir, 'tdf169019-tmp.ods')

            with self.ui_test.load_file(get_url_for_data_file('tdf169019.ods')):

                with self.ui_test.execute_dialog_through_command(".uno:DataProvider") as xDialog:
                    xDB = xDialog.getChild("select_db_range")
                    select_by_text(xDB, "myImport")
                    self.assertEqual("myImport", get_state_as_dict(xDB)['DisplayText'])

                    xBrowse = xDialog.getChild("browse")
                    with self.ui_test.execute_blocking_action(
                            xBrowse.executeAction, args=('CLICK', ()), close_button="") as dialog:
                        xFileName = dialog.getChild("file_name")
                        xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf169019.csv")}))
                        xOpen = dialog.getChild("open")
                        xOpen.executeAction("CLICK", tuple())

                    xProvider = xDialog.getChild("provider_lst")
                    xURL = xDialog.getChild("ed_url")
                    self.assertEqual(get_url_for_data_file("tdf169019.csv"), get_state_as_dict(xURL)['Text'])
                    self.assertEqual("CSV", get_state_as_dict(xProvider)['DisplayText'])

                    xTransformation = xDialog.getChild("transformation_box")
                    select_by_text(xTransformation, "Split Column")
                    self.assertEqual("Split Column", get_state_as_dict(xTransformation)['DisplayText'])

                    xAdd = xDialog.getChild("add_transformation")
                    xAdd.executeAction("CLICK", tuple())

                    xSeparator1 = xDialog.getChild("ed_separator0")
                    xNumCols1 = xDialog.getChild("num_cols0")
                    xSeparator1.executeAction("TYPE", mkPropertyValues({"TEXT":"|"}))
                    xNumCols1.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))

                    xApply = xDialog.getChild("apply")
                    xApply.executeAction("CLICK", tuple())

                    self.assertEqual("Split Column", get_state_as_dict(xTransformation)['DisplayText'])

                    xAdd = xDialog.getChild("add_transformation")
                    xAdd.executeAction("CLICK", tuple())

                    xSeparator2 = xDialog.getChild("ed_separator1")
                    xNumCols2 = xDialog.getChild("num_cols1")
                    xSeparator2.executeAction("TYPE", mkPropertyValues({"TEXT":"|"}))
                    xNumCols2.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))

                    xApply = xDialog.getChild("apply")
                    xApply.executeAction("CLICK", tuple())

                # Save Copy as
                with self.ui_test.execute_dialog_through_command('.uno:SaveAs', close_button="open") as xDialog:
                    xFileName = xDialog.getChild('file_name')
                    xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'CTRL+A'}))
                    xFileName.executeAction('TYPE', mkPropertyValues({'KEYCODE':'BACKSPACE'}))
                    xFileName.executeAction('TYPE', mkPropertyValues({'TEXT': xFilePath}))

            with self.ui_test.load_file(systemPathToFileUrl(xFilePath)) as doc2:
                # Without the fix in place, this test would have failed with
                # AssertionError: 'Rank' != 'Rank|Males|Females\r'
                self.assertEqual("Rank", get_cell_by_position(doc2, 0, 0, 0).getString())
                self.assertEqual("Males", get_cell_by_position(doc2, 0, 1, 0).getString())
                self.assertEqual("Females\r", get_cell_by_position(doc2, 0, 2, 0).getString())
                for i in range(1, 101):
                    self.assertEqual(str(i), get_cell_by_position(doc2, 0, 0, i).getString())

    def test_insert_correct_sheet(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf169817.ods')) as document:

            with self.ui_test.execute_dialog_through_command(".uno:DataProvider") as xDialog:
                xDB = xDialog.getChild("select_db_range")
                xURL = xDialog.getChild("ed_url")
                xProvider = xDialog.getChild("provider_lst")
                select_by_text(xDB, "Second")
                self.assertEqual("Second", get_state_as_dict(xDB)['DisplayText'])

                xBrowse = xDialog.getChild("browse")
                with self.ui_test.execute_blocking_action(
                        xBrowse.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    xFileName = dialog.getChild("file_name")
                    xFileName.executeAction("TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf169817.csv")}))
                    xOpen = dialog.getChild("open")
                    xOpen.executeAction("CLICK", tuple())

                self.assertEqual(get_url_for_data_file("tdf169817.csv"), get_state_as_dict(xURL)['Text'])
                self.assertEqual("CSV", get_state_as_dict(xProvider)['DisplayText'])

            # Without the fix in place, this test would have failed with
            # AssertionError: 'ID' != ''
            self.assertEqual("ID", get_cell_by_position(document, 1, 7, 4).getString())
            self.assertEqual("Region", get_cell_by_position(document, 1, 8, 4).getString())
            self.assertEqual("Name", get_cell_by_position(document, 1, 9, 4).getString())
            self.assertEqual("Sales", get_cell_by_position(document, 1, 10, 4).getString())

    def test_consider_labels_when_sorting(self):

        with self.ui_test.load_file(get_url_for_data_file('tdf169049_WithLabel.ods')) as document:

            with self.ui_test.execute_dialog_through_command(".uno:DataProvider") as xDialog:
                xDB = xDialog.getChild("select_db_range")
                select_by_text(xDB, "withheader")
                self.assertEqual("withheader", get_state_as_dict(xDB)['DisplayText'])

                xProvider = xDialog.getChild("provider_lst")
                select_by_text(xProvider, "CSV")
                self.assertEqual("CSV", get_state_as_dict(xProvider)['DisplayText'])

                xURL = xDialog.getChild("ed_url")
                xBrowse = xDialog.getChild("browse")
                with self.ui_test.execute_blocking_action(
                        xBrowse.executeAction, args=('CLICK', ()), close_button="") as dialog:
                    xFileName = dialog.getChild("file_name")
                    xFileName.executeAction(
                        "TYPE", mkPropertyValues({"TEXT": get_url_for_data_file("tdf169049_Source.csv")}))
                    xOpen = dialog.getChild("open")
                    xOpen.executeAction("CLICK", tuple())
                self.assertEqual(get_url_for_data_file("tdf169049_Source.csv"),
                                 get_state_as_dict(xURL)['Text'])

                xTransformation = xDialog.getChild("transformation_box")
                select_by_text(xTransformation, "Sort Columns")
                self.assertEqual("Sort Columns",
                                 get_state_as_dict(xTransformation)['DisplayText'])

                xAdd = xDialog.getChild("add_transformation")
                xAdd.executeAction("CLICK", tuple())
                xAscending = xDialog.getChild("ed_ascending")
                select_by_text(xAscending, "Ascending Order")
                self.assertEqual("Ascending Order", get_state_as_dict(xAscending)['DisplayText'])

                xByColumn = xDialog.getChild("ed_columns")
                xByColumn.executeAction("TYPE", mkPropertyValues({"TEXT":"3"}))

                # The transformation is only performed after clicking the Apply button.
                xApply = xDialog.getChild("apply")
                xApply.executeAction("CLICK", tuple())

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Name' != 'Ali'
            self.assertEqual("Name", get_cell_by_position(document, 0, 3, 1).getString())
            self.assertEqual("Ali", get_cell_by_position(document, 0, 3, 2).getString())
            self.assertEqual("Frieda", get_cell_by_position(document, 0, 3, 9).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
