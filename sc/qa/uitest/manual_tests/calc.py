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
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file, type_text, select_pos
from uitest.uihelper.calc import enter_text_to_cell

class ManualCalcTests(UITestCase):

    # http://manual-test.libreoffice.org/manage/case/189/
    def test_define_database_range(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            # Select range A1:D10
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D10"}))

            # Execute "Define DB Range dialog"
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DefineDBName") as xDefineNameDlg:


                xEntryBox = xDefineNameDlg.getChild("entry")
                type_text(xEntryBox, "my_database")


            # Deselect range
            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            # Execute "Select DB Range dialog"
            with self.ui_test.execute_dialog_through_command(".uno:SelectDB") as xSelectNameDlg:

                xListBox = xSelectNameDlg.getChild("treeview")
                xListBoxState = get_state_as_dict(xListBox)
                self.assertEqual(xListBoxState["SelectionCount"], "1")
                self.assertEqual(xListBoxState["SelectEntryText"], "my_database")

            # Assert that the correct range has been selected
            gridWinState = get_state_as_dict(xGridWin)
            self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.D10")


    # http://manual-test.libreoffice.org/manage/case/190/
    def test_sort_data(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:

            # Insert data
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            enter_text_to_cell(xGridWin, "B1", "3")
            enter_text_to_cell(xGridWin, "B2", "25")
            enter_text_to_cell(xGridWin, "B3", "17")
            enter_text_to_cell(xGridWin, "B4", "9")
            enter_text_to_cell(xGridWin, "B5", "19")
            enter_text_to_cell(xGridWin, "B6", "0")
            enter_text_to_cell(xGridWin, "B7", "107")
            enter_text_to_cell(xGridWin, "B8", "89")
            enter_text_to_cell(xGridWin, "B9", "8")
            enter_text_to_cell(xGridWin, "B10", "33")

            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B10"}))

            # Execute "Sort" dialog
            with self.ui_test.execute_dialog_through_command(".uno:DataSort"):
                pass


            self.assertEqual(get_cell_by_position(document, 0, 1, 0).getValue(), 0)
            self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 3)
            self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 8)
            self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 9)
            self.assertEqual(get_cell_by_position(document, 0, 1, 4).getValue(), 17)
            self.assertEqual(get_cell_by_position(document, 0, 1, 5).getValue(), 19)
            self.assertEqual(get_cell_by_position(document, 0, 1, 6).getValue(), 25)
            self.assertEqual(get_cell_by_position(document, 0, 1, 7).getValue(), 33)
            self.assertEqual(get_cell_by_position(document, 0, 1, 8).getValue(), 89)
            self.assertEqual(get_cell_by_position(document, 0, 1, 9).getValue(), 107)


    # http://manual-test.libreoffice.org/manage/case/191/
    def test_validation(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C10"}))

            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xValidationDlg:

                xAllowList = xValidationDlg.getChild("allow")
                select_pos(xAllowList, "1")

                xData = xValidationDlg.getChild("data")
                select_pos(xData, "5")

                xVal = xValidationDlg.getChild("max")
                xVal.executeAction("TYPE", mkPropertyValues({"TEXT":"0"}))


            def enter_text(cell, text):
                enter_text_to_cell(xGridWin, cell, text)

            with self.ui_test.execute_blocking_action(enter_text, args=("A1", "abc")):
                pass
            with self.ui_test.execute_blocking_action(enter_text, args=("B6", "2.18")):
                pass

            enter_text_to_cell(xGridWin, "C2", "24")


    # http://manual-test.libreoffice.org/manage/case/187/
    def test_transpose(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            enter_text_to_cell(xGridWin, "B3", "abcd")
            enter_text_to_cell(xGridWin, "B4", "edfg")
            enter_text_to_cell(xGridWin, "C3", "35")
            enter_text_to_cell(xGridWin, "C4", "5678")

            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C10"}))

            self.xUITest.executeCommand(".uno:Cut")

            xGridWin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:PasteSpecial") as xPasteSpecialDlg:

                xAllChkBox = xPasteSpecialDlg.getChild("paste_all")
                xAllChkBox.executeAction("CLICK", tuple())

                xTransposeChkBox = xPasteSpecialDlg.getChild("transpose")
                xTransposeChkBox.executeAction("CLICK", tuple())


            self.assertEqual(get_cell_by_position(document, 0, 2, 1).getString(), "abcd")
            self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 35)
            self.assertEqual(get_cell_by_position(document, 0, 3, 1).getString(), "edfg")
            self.assertEqual(get_cell_by_position(document, 0, 3, 2).getValue(), 5678)


    # http://manual-test.libreoffice.org/manage/case/151/
    def test_cell_recalc(self):
        with self.ui_test.load_file(get_url_for_data_file("cell_recalc.ods")) as document:

            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")
            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "D2:D9"}))
            self.xUITest.executeCommand(".uno:Cut")

            self.assertEqual(get_cell_by_position(document, 0, 3, 15).getValue(), 0)

            self.xUITest.executeCommand(".uno:Undo")

            for i in range(1, 9):
                self.assertTrue(get_cell_by_position(document, 0, 3, i).getValue() != 0)

            self.assertEqual(get_cell_by_position(document, 0, 3, 15).getValue(), 195)

    # http://manual-test.libreoffice.org/manage/case/143/
    def test_random_numbers(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xGridWin = self.xUITest.getTopFocusWindow().getChild("grid_window")

            xGridWin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A10"}))

            with self.ui_test.execute_modeless_dialog_through_command(".uno:RandomNumberGeneratorDialog") as xRandomNumberDlg:
                xDistributionLstBox = xRandomNumberDlg.getChild("distribution-combo")
                select_pos(xDistributionLstBox, "1")

                xMin = xRandomNumberDlg.getChild("parameter1-spin")
                xMin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                xMin.executeAction("TYPE", mkPropertyValues({"TEXT": "-2"}))
                xMax = xRandomNumberDlg.getChild("parameter2-spin")
                xMax.executeAction("TYPE", mkPropertyValues({"KEYCODE": "CTRL+A"}))
                xMax.executeAction("TYPE", mkPropertyValues({"TEXT": "10"}))

                xApplyBtn = xRandomNumberDlg.getChild("apply")
                xApplyBtn.executeAction("CLICK", tuple())


                def check_random_values():
                    for i in range(1, 9):
                        val = get_cell_by_position(document, 0, 0, i).getValue()
                        self.assertTrue(val <= 10 and val >= -2)

                check_random_values()


            # we might want to check that clicking 'ok' actually changes the values
            check_random_values()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
