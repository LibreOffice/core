# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tTest(UITestCase):
    def test_fTest_column(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #fill data
        enter_text_to_cell(gridwin, "A1", "28")
        enter_text_to_cell(gridwin, "A2", "26")
        enter_text_to_cell(gridwin, "A3", "31")
        enter_text_to_cell(gridwin, "A4", "23")
        enter_text_to_cell(gridwin, "A5", "20")
        enter_text_to_cell(gridwin, "A6", "27")
        enter_text_to_cell(gridwin, "A7", "28")
        enter_text_to_cell(gridwin, "A8", "14")
        enter_text_to_cell(gridwin, "A9", "4")
        enter_text_to_cell(gridwin, "A10", "0")
        enter_text_to_cell(gridwin, "A11", "2")
        enter_text_to_cell(gridwin, "A12", "8")
        enter_text_to_cell(gridwin, "A13", "9")

        enter_text_to_cell(gridwin, "B1", "19")
        enter_text_to_cell(gridwin, "B2", "13")
        enter_text_to_cell(gridwin, "B3", "12")
        enter_text_to_cell(gridwin, "B4", "5")
        enter_text_to_cell(gridwin, "B5", "34")
        enter_text_to_cell(gridwin, "B6", "31")
        enter_text_to_cell(gridwin, "B7", "31")
        enter_text_to_cell(gridwin, "B8", "12")
        enter_text_to_cell(gridwin, "B9", "24")
        enter_text_to_cell(gridwin, "B10", "23")
        enter_text_to_cell(gridwin, "B11", "19")
        enter_text_to_cell(gridwin, "B12", "10")
        enter_text_to_cell(gridwin, "B13", "33")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B13"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:FTestDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xvariable1rangeedit = xDialog.getChild("variable1-range-edit")
        xvariable2rangeedit = xDialog.getChild("variable2-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")

        xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvariable1rangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$A$13"}))
        xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvariable2rangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$B$1:$B$13"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"F1"}))
        xgroupedbycolumnsradio.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "F-test")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getString(), "Alpha")

        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getString(), "Mean")
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getString(), "Variance")
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getString(), "Observations")
        self.assertEqual(get_cell_by_position(document, 0, 5, 6).getString(), "df")
        self.assertEqual(get_cell_by_position(document, 0, 5, 7).getString(), "F")
        self.assertEqual(get_cell_by_position(document, 0, 5, 8).getString(), "P (F<=f) right-tail")
        self.assertEqual(get_cell_by_position(document, 0, 5, 9).getString(), "F Critical right-tail")
        self.assertEqual(get_cell_by_position(document, 0, 5, 10).getString(), "P (F<=f) left-tail")
        self.assertEqual(get_cell_by_position(document, 0, 5, 11).getString(), "F Critical left-tail")
        self.assertEqual(get_cell_by_position(document, 0, 5, 12).getString(), "P two-tail")
        self.assertEqual(get_cell_by_position(document, 0, 5, 13).getString(), "F Critical two-tail")


        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0.05)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getString(), "Variable 1")
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 3).getValue(),12), 16.923076923077)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 4).getValue(),12), 125.076923076923)
        self.assertEqual(get_cell_by_position(document, 0, 6, 5).getValue(), 13)
        self.assertEqual(get_cell_by_position(document, 0, 6, 6).getValue(), 12)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 7).getValue(),12), 1.324463752376)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 8).getValue(),12), 0.317061414639)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 9).getValue(),12), 2.686637112496)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 10).getValue(),12), 0.682938585361)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 12).getValue(),12), 0.634122829279)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 13).getValue(),12), 0.305131354874)

        self.assertEqual(get_cell_by_position(document, 0, 7, 2).getString(), "Variable 2")
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 3).getValue(),12), 20.461538461538)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 4).getValue(),12), 94.435897435897)
        self.assertEqual(get_cell_by_position(document, 0, 7, 5).getValue(), 13)
        self.assertEqual(get_cell_by_position(document, 0, 7, 6).getValue(), 12)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 13).getValue(),12), 3.277277094033)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:FTestDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
