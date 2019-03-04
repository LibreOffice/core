# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import org.libreoffice.unotest
import os
import pathlib
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#AutoSum feature test
def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

class calcAutosum(UITestCase):

    def test_autosum_test1(self):
        #Sum on range and Sum on Sum's
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Place the cell cursor on the gray cells located under Point 1.
        #(Multiselection is not possible at this place) and press the Sum Icon in the formula bar.
        #Now hit the enter key and the result should be shown. Do so for each gray cell in this part of the document.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B10"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B13"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B14"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        self.assertEqual(get_cell_by_position(document, 0, 1, 9).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 9).getFormula(), "=SUM(B8:B9)")
        self.assertEqual(get_cell_by_position(document, 0, 1, 12).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 12).getFormula(), "=SUM(B11:B12)")
        self.assertEqual(get_cell_by_position(document, 0, 1, 13).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 1, 13).getFormula(), "=SUM(B13:B13;B10:B10)")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "F8:F14"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        self.assertEqual(get_cell_by_position(document, 0, 5, 13).getValue(), 4)
        self.assertEqual(get_cell_by_position(document, 0, 5, 13).getFormula(), "=SUM(F13:F13;F10:F10)")

        self.ui_test.close_doc()

    def test_autosum_test2(self):
        #Sum on Row and Column
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E25"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E26"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E27"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E28"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E29"}))
        self.xUITest.executeCommand(".uno:AutoSum")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E30"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 4, 24).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 4, 24).getFormula(), "=SUM(E22:E24)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 25).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 4, 25).getFormula(), "=SUM(B26:D26)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 26).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 4, 26).getFormula(), "=SUM(B27:D27)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 27).getValue(), 3)
        self.assertEqual(get_cell_by_position(document, 0, 4, 27).getFormula(), "=SUM(B28:D28)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 28).getValue(), 9)
        self.assertEqual(get_cell_by_position(document, 0, 4, 28).getFormula(), "=SUM(E26:E28)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 29).getValue(), 12)
        self.assertEqual(get_cell_by_position(document, 0, 4, 29).getFormula(), "=SUM(E29:E29;E25:E25)")

        self.ui_test.close_doc()

    def test_autosum_test3(self):
        #Subtotals on Autosum
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C49"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 2, 48).getValue(), 20)
        self.assertEqual(get_cell_by_position(document, 0, 2, 48).getFormula(), "=SUBTOTAL(9;C38:C48)")

        self.ui_test.close_doc()

    def test_autosum_test4(self):
        #Autosum on column with selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B59:B64"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 1, 63).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 1, 63).getFormula(), "=SUM(B59:B63)")

        self.ui_test.close_doc()

    def test_autosum_test5(self):
        #5.Autosum on rows with selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B76:E80"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 4, 75).getValue(), 30)
        self.assertEqual(get_cell_by_position(document, 0, 4, 75).getFormula(), "=SUM(B76:D76)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 76).getValue(), 60)
        self.assertEqual(get_cell_by_position(document, 0, 4, 76).getFormula(), "=SUM(B77:D77)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 77).getValue(), 90)
        self.assertEqual(get_cell_by_position(document, 0, 4, 77).getFormula(), "=SUM(B78:D78)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 78).getValue(), 120)
        self.assertEqual(get_cell_by_position(document, 0, 4, 78).getFormula(), "=SUM(B79:D79)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 79).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 4, 79).getFormula(), "=SUM(B80:D80)")

        self.ui_test.close_doc()

    def test_autosum_test6(self):
        #6.Subtotal on column with selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C92:C101"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 2, 100).getValue(), 19)
        self.assertEqual(get_cell_by_position(document, 0, 2, 100).getFormula(), "=SUBTOTAL(9;C92:C100)")

        self.ui_test.close_doc()

    def test_autosum_test7(self):
        #7.Autosum on column without selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B109:B113"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 1, 113).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 1, 113).getFormula(), "=SUM(B109:B113)")

        self.ui_test.close_doc()

    def test_autosum_test8(self):
        #8.Autosum on rows without selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B126:D126"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B128:D128", "EXTEND":"1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B130:D130", "EXTEND":"1"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 4, 125).getValue(), 30)
        self.assertEqual(get_cell_by_position(document, 0, 4, 125).getFormula(), "=SUM(B126:D126)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 127).getValue(), 90)
        self.assertEqual(get_cell_by_position(document, 0, 4, 127).getFormula(), "=SUM(B128:D128)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 129).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 4, 129).getFormula(), "=SUM(B130:D130)")

        self.ui_test.close_doc()

    def test_autosum_test9(self):
        #9.Subtotal on column without selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C142:C149"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 2, 150).getValue(), 19)
        self.assertEqual(get_cell_by_position(document, 0, 2, 150).getFormula(), "=SUBTOTAL(9;C142:C149)")

        self.ui_test.close_doc()

    def test_autosum_test10(self):
        #10.Autosum on multiselected columns without selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B160:D164"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 1, 164).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 1, 164).getFormula(), "=SUM(B160:B164)")
        self.assertEqual(get_cell_by_position(document, 0, 2, 164).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 2, 164).getFormula(), "=SUM(C160:C164)")
        self.assertEqual(get_cell_by_position(document, 0, 3, 164).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 3, 164).getFormula(), "=SUM(D160:D164)")
        self.ui_test.close_doc()

    def test_autosum_test11(self):
        #11.Autosum on columns with formula results without selected empty cell for result
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B173:D177"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(round(get_cell_by_position(document, 0, 1, 177).getValue(),2), 2.55)
        self.assertEqual(get_cell_by_position(document, 0, 1, 177).getFormula(), "=SUM(B173:B177)")
        self.assertEqual(round(get_cell_by_position(document, 0, 2, 177).getValue(),2), -4.91)
        self.assertEqual(get_cell_by_position(document, 0, 2, 177).getFormula(), "=SUM(C173:C177)")
        self.assertEqual(get_cell_by_position(document, 0, 3, 177).getValue(), 5500)
        self.assertEqual(get_cell_by_position(document, 0, 3, 177).getFormula(), "=SUM(D173:D177)")
        self.ui_test.close_doc()

    def test_autosum_test12(self):
        #12.Autosum on column with filled cell under selected area
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B186:D190"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 1, 191).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 1, 191).getFormula(), "=SUM(B186:B190)")
        self.assertEqual(get_cell_by_position(document, 0, 2, 191).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 2, 191).getFormula(), "=SUM(C186:C190)")
        self.assertEqual(get_cell_by_position(document, 0, 3, 191).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 3, 191).getFormula(), "=SUM(D186:D190)")
        self.ui_test.close_doc()

    def test_autosum_test13(self):
        #13.Autosum on column and rows with empty cells selected for row and column
        calc_doc = self.ui_test.load_file(get_url_for_data_file("autosum.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B203:E208"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 1, 207).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 1, 207).getFormula(), "=SUM(B203:B207)")
        self.assertEqual(get_cell_by_position(document, 0, 2, 207).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 2, 207).getFormula(), "=SUM(C203:C207)")
        self.assertEqual(get_cell_by_position(document, 0, 3, 207).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 3, 207).getFormula(), "=SUM(D203:D207)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 207).getValue(), 450)
        self.assertEqual(get_cell_by_position(document, 0, 4, 207).getFormula(), "=SUM(B208:D208)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 202).getValue(), 30)
        self.assertEqual(get_cell_by_position(document, 0, 4, 202).getFormula(), "=SUM(B203:D203)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 203).getValue(), 60)
        self.assertEqual(get_cell_by_position(document, 0, 4, 203).getFormula(), "=SUM(B204:D204)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 204).getValue(), 90)
        self.assertEqual(get_cell_by_position(document, 0, 4, 204).getFormula(), "=SUM(B205:D205)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 205).getValue(), 120)
        self.assertEqual(get_cell_by_position(document, 0, 4, 205).getFormula(), "=SUM(B206:D206)")
        self.assertEqual(get_cell_by_position(document, 0, 4, 206).getValue(), 150)
        self.assertEqual(get_cell_by_position(document, 0, 4, 206).getFormula(), "=SUM(B207:D207)")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: