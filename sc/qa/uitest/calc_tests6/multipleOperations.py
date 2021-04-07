# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell

#https://www.linuxtopia.org/online_books/office_guides/openoffice_3_calc_user_guide/openoffice_calc_Multiple_operations_Multiple_operations_in_columns_or_rows.html

class multipleOperations(UITestCase):

   def test_multiple_operations_one_variable(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #enter data
        enter_text_to_cell(xGridWindow, "B1", "10")
        enter_text_to_cell(xGridWindow, "B2", "2")
        enter_text_to_cell(xGridWindow, "B3", "10000")
        enter_text_to_cell(xGridWindow, "B4", "2000")
        enter_text_to_cell(xGridWindow, "B5", "=B4*(B1-B2)-B3")

        enter_text_to_cell(xGridWindow, "D2", "500")
        enter_text_to_cell(xGridWindow, "D3", "1000")
        enter_text_to_cell(xGridWindow, "D4", "1500")
        enter_text_to_cell(xGridWindow, "D5", "2000")
        enter_text_to_cell(xGridWindow, "D6", "2500")
        enter_text_to_cell(xGridWindow, "D7", "3000")
        enter_text_to_cell(xGridWindow, "D8", "3500")
        enter_text_to_cell(xGridWindow, "D9", "4000")
        enter_text_to_cell(xGridWindow, "D10", "4500")
        enter_text_to_cell(xGridWindow, "D11", "5000")
        #Select the range D2:E11
        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "D2:E11"}))
        #Choose Data > Multiple Operations.
        self.ui_test.execute_modeless_dialog_through_command(".uno:TableOperationDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #Formulas = B5 ; Column input cell = B4
        formulas = xDialog.getChild("formulas")
        col = xDialog.getChild("col")
        formulas.executeAction("TYPE", mkPropertyValues({"TEXT":"$B$5"}))
        col.executeAction("TYPE", mkPropertyValues({"TEXT":"$B$4"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), -6000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), -2000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 2000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 6000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 5).getValue(), 10000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 6).getValue(), 14000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 7).getValue(), 18000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 8).getValue(), 22000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 9).getValue(), 26000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 10).getValue(), 30000)

        self.ui_test.close_doc()
   def test_multiple_operations_several_formulas(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #enter data
        enter_text_to_cell(xGridWindow, "B1", "10")
        enter_text_to_cell(xGridWindow, "B2", "2")
        enter_text_to_cell(xGridWindow, "B3", "10000")
        enter_text_to_cell(xGridWindow, "B4", "2000")
        enter_text_to_cell(xGridWindow, "B5", "=B4*(B1-B2)-B3")
        enter_text_to_cell(xGridWindow, "C5", "=B5/B4")

        enter_text_to_cell(xGridWindow, "D2", "500")
        enter_text_to_cell(xGridWindow, "D3", "1000")
        enter_text_to_cell(xGridWindow, "D4", "1500")
        enter_text_to_cell(xGridWindow, "D5", "2000")
        enter_text_to_cell(xGridWindow, "D6", "2500")
        enter_text_to_cell(xGridWindow, "D7", "3000")
        enter_text_to_cell(xGridWindow, "D8", "3500")
        enter_text_to_cell(xGridWindow, "D9", "4000")
        enter_text_to_cell(xGridWindow, "D10", "4500")
        enter_text_to_cell(xGridWindow, "D11", "5000")
        #Select the range D2:F11
        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "D2:F11"}))
        #Choose Data > Multiple Operations.
        self.ui_test.execute_modeless_dialog_through_command(".uno:TableOperationDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        #Formulas = B5 ; Column input cell = B4
        formulas = xDialog.getChild("formulas")
        col = xDialog.getChild("col")
        formulas.executeAction("TYPE", mkPropertyValues({"TEXT":"$B$5:$C$5"}))
        col.executeAction("TYPE", mkPropertyValues({"TEXT":"$B$4"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.assertEqual(get_cell_by_position(document, 0, 4, 1).getValue(), -6000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 2).getValue(), -2000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 3).getValue(), 2000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 4).getValue(), 6000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 5).getValue(), 10000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 6).getValue(), 14000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 7).getValue(), 18000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 8).getValue(), 22000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 9).getValue(), 26000)
        self.assertEqual(get_cell_by_position(document, 0, 4, 10).getValue(), 30000)

        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), -12)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue(), -2)
        self.assertEqual(round(get_cell_by_position(document, 0, 5, 3).getValue(),2), 1.33)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getValue(), 4)
        self.assertEqual(round(get_cell_by_position(document, 0, 5, 6).getValue(),2), 4.67)
        self.assertEqual(round(get_cell_by_position(document, 0, 5, 7).getValue(),2), 5.14)
        self.assertEqual(get_cell_by_position(document, 0, 5, 8).getValue(), 5.5)
        self.assertEqual(round(get_cell_by_position(document, 0, 5, 9).getValue(),2), 5.78)
        self.assertEqual(get_cell_by_position(document, 0, 5, 10).getValue(), 6)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
