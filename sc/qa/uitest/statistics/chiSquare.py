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

class chiSquare(UITestCase):
    def test_chiSquare(self):
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
        self.ui_test.execute_modeless_dialog_through_command(".uno:ChiSquareTestDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xinputrangeedit = xDialog.getChild("input-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")

        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$B$13"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"F1"}))
        xgroupedbycolumnsradio.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "Test of Independence (Chi-Square)")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getString(), "Alpha")

        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getString(), "df")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getString(), "P-value")
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getString(), "Test Statistic")
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getString(), "Critical Value")

        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0.05)
        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue(), 12)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 4).getValue(),12), 91.687005584173)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 5).getValue(),12), 21.026069817483)

        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:ChiSquareTestDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
