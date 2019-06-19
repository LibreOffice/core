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

class covariance(UITestCase):
    def test_statistic_covariance_column(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #fill data
        enter_text_to_cell(gridwin, "A1", "Maths")
        enter_text_to_cell(gridwin, "A2", "47")
        enter_text_to_cell(gridwin, "A3", "36")
        enter_text_to_cell(gridwin, "A4", "40")
        enter_text_to_cell(gridwin, "A5", "39")
        enter_text_to_cell(gridwin, "A7", "47")
        enter_text_to_cell(gridwin, "A8", "29")
        enter_text_to_cell(gridwin, "A9", "27")
        enter_text_to_cell(gridwin, "A10", "57")
        enter_text_to_cell(gridwin, "A11", "56")
        enter_text_to_cell(gridwin, "A12", "57")
        enter_text_to_cell(gridwin, "A13", "26")

        enter_text_to_cell(gridwin, "B1", "Physics")
        enter_text_to_cell(gridwin, "B2", "67")
        enter_text_to_cell(gridwin, "B3", "68")
        enter_text_to_cell(gridwin, "B4", "65")
        enter_text_to_cell(gridwin, "B5", "64")
        enter_text_to_cell(gridwin, "B6", "38")
        enter_text_to_cell(gridwin, "B7", "84")
        enter_text_to_cell(gridwin, "B8", "80")
        enter_text_to_cell(gridwin, "B9", "49")
        enter_text_to_cell(gridwin, "B10", "49")
        enter_text_to_cell(gridwin, "B11", "33")

        enter_text_to_cell(gridwin, "C1", "Biology")
        enter_text_to_cell(gridwin, "C2", "33")
        enter_text_to_cell(gridwin, "C3", "42")
        enter_text_to_cell(gridwin, "C4", "44")
        enter_text_to_cell(gridwin, "C5", "60")
        enter_text_to_cell(gridwin, "C6", "43")
        enter_text_to_cell(gridwin, "C7", "62")
        enter_text_to_cell(gridwin, "C8", "51")
        enter_text_to_cell(gridwin, "C9", "40")
        enter_text_to_cell(gridwin, "C10", "12")
        enter_text_to_cell(gridwin, "C11", "60")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C13"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:CovarianceDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xinputrangeedit = xDialog.getChild("input-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")

        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$C$13"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$F$1"}))
        xgroupedbycolumnsradio.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "Covariances")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getString(), "Column 1")
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getString(), "Column 2")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getString(), "Column 3")

        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString(), "Column 1")
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 1).getValue(),11), 126.80991735537)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 2).getValue(),12), -61.444444444444)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 3).getValue()), -32)

        self.assertEqual(get_cell_by_position(document, 0, 7, 0).getString(), "Column 2")
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 2).getValue(),2), 258.41)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 3).getValue(),2), 53.11)

        self.assertEqual(get_cell_by_position(document, 0, 8, 0).getString(), "Column 3")
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 3).getValue(),2), 204.61)

        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:CovarianceDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_statistic_covariance_row(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #fill data
        enter_text_to_cell(gridwin, "A1", "Maths")
        enter_text_to_cell(gridwin, "A2", "Physics")
        enter_text_to_cell(gridwin, "A3", "Biology")

        enter_text_to_cell(gridwin, "B1", "47")
        enter_text_to_cell(gridwin, "B2", "67")
        enter_text_to_cell(gridwin, "B3", "33")

        enter_text_to_cell(gridwin, "C1", "36")
        enter_text_to_cell(gridwin, "C2", "68")
        enter_text_to_cell(gridwin, "C3", "42")

        enter_text_to_cell(gridwin, "D1", "40")
        enter_text_to_cell(gridwin, "D2", "65")
        enter_text_to_cell(gridwin, "D3", "44")

        enter_text_to_cell(gridwin, "E1", "39")
        enter_text_to_cell(gridwin, "E2", "64")
        enter_text_to_cell(gridwin, "E3", "60")

        enter_text_to_cell(gridwin, "F2", "38")
        enter_text_to_cell(gridwin, "F3", "43")

        enter_text_to_cell(gridwin, "G1", "47")
        enter_text_to_cell(gridwin, "G2", "84")
        enter_text_to_cell(gridwin, "G3", "62")

        enter_text_to_cell(gridwin, "H1", "29")
        enter_text_to_cell(gridwin, "H2", "80")
        enter_text_to_cell(gridwin, "H3", "51")

        enter_text_to_cell(gridwin, "I1", "27")
        enter_text_to_cell(gridwin, "I2", "49")
        enter_text_to_cell(gridwin, "I3", "40")

        enter_text_to_cell(gridwin, "J1", "57")
        enter_text_to_cell(gridwin, "J2", "49")
        enter_text_to_cell(gridwin, "J3", "12")

        enter_text_to_cell(gridwin, "K1", "56")
        enter_text_to_cell(gridwin, "K2", "33")
        enter_text_to_cell(gridwin, "K3", "60")

        enter_text_to_cell(gridwin, "L1", "57")

        enter_text_to_cell(gridwin, "M1", "26")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:M3"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:CovarianceDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xinputrangeedit = xDialog.getChild("input-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")

        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$M$3"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$A$7"}))
        xgroupedbyrowsradio.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 6).getString(), "Covariances")
        self.assertEqual(get_cell_by_position(document, 0, 0, 7).getString(), "Row 1")
        self.assertEqual(get_cell_by_position(document, 0, 0, 8).getString(), "Row 2")
        self.assertEqual(get_cell_by_position(document, 0, 0, 9).getString(), "Row 3")

        self.assertEqual(get_cell_by_position(document, 0, 1, 6).getString(), "Row 1")
        self.assertEqual(round(get_cell_by_position(document, 0, 1, 7).getValue(),10), 126.8099173554)
        self.assertEqual(round(get_cell_by_position(document, 0, 1, 8).getValue(),12), -61.444444444444)
        self.assertEqual(round(get_cell_by_position(document, 0, 1, 9).getValue()), -32)

        self.assertEqual(get_cell_by_position(document, 0, 2, 6).getString(), "Row 2")
        self.assertEqual(round(get_cell_by_position(document, 0, 2, 8).getValue(),2), 258.41)
        self.assertEqual(round(get_cell_by_position(document, 0, 2, 9).getValue(),2), 53.11)

        self.assertEqual(get_cell_by_position(document, 0, 3, 6).getString(), "Row 3")
        self.assertEqual(round(get_cell_by_position(document, 0, 3, 9).getValue(),2), 204.61)

        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
