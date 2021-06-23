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

class exponentialSmoothing(UITestCase):
    def test_exponential_smoothing_column(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #fill data
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "0")
        enter_text_to_cell(gridwin, "A3", "0")
        enter_text_to_cell(gridwin, "A4", "0")
        enter_text_to_cell(gridwin, "A5", "0")
        enter_text_to_cell(gridwin, "A7", "0")
        enter_text_to_cell(gridwin, "A8", "0")
        enter_text_to_cell(gridwin, "A9", "0")
        enter_text_to_cell(gridwin, "A10", "0")
        enter_text_to_cell(gridwin, "A11", "0")
        enter_text_to_cell(gridwin, "A12", "0")
        enter_text_to_cell(gridwin, "A13", "0")

        enter_text_to_cell(gridwin, "B1", "0")
        enter_text_to_cell(gridwin, "B2", "0")
        enter_text_to_cell(gridwin, "B3", "1")
        enter_text_to_cell(gridwin, "B4", "0")
        enter_text_to_cell(gridwin, "B5", "0")
        enter_text_to_cell(gridwin, "B6", "0")
        enter_text_to_cell(gridwin, "B7", "0")
        enter_text_to_cell(gridwin, "B8", "0")
        enter_text_to_cell(gridwin, "B9", "0")
        enter_text_to_cell(gridwin, "B10", "0")
        enter_text_to_cell(gridwin, "B11", "0")
        enter_text_to_cell(gridwin, "B12", "0")
        enter_text_to_cell(gridwin, "B13", "0")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B13"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:ExponentialSmoothingDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xinputrangeedit = xDialog.getChild("input-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")
        xmoothingfactorspin = xDialog.getChild("smoothing-factor-spin")

        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$B$13"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$F$1"}))
        xmoothingfactorspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xmoothingfactorspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xmoothingfactorspin.executeAction("TYPE", mkPropertyValues({"TEXT":"0.5"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "Alpha")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 0.5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getString(), "Column 1")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getValue(), 0.5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 6).getValue(), 0.25)
        self.assertEqual(get_cell_by_position(document, 0, 5, 7).getValue(), 0.125)
        self.assertEqual(get_cell_by_position(document, 0, 5, 8).getValue(), 0.0625)
        self.assertEqual(get_cell_by_position(document, 0, 5, 9).getValue(), 0.03125)
        self.assertEqual(get_cell_by_position(document, 0, 5, 10).getValue(), 0.015625)
        self.assertEqual(get_cell_by_position(document, 0, 5, 11).getValue(), 0.0078125)
        self.assertEqual(get_cell_by_position(document, 0, 5, 12).getValue(), 0.00390625)
        self.assertEqual(get_cell_by_position(document, 0, 5, 13).getValue(), 0.001953125)
        self.assertEqual(get_cell_by_position(document, 0, 5, 14).getValue(), 0.0009765625)
        self.assertEqual(get_cell_by_position(document, 0, 5, 15).getValue(), 0.00048828125)
        self.assertEqual(get_cell_by_position(document, 0, 5, 16).getValue(), 0.000244140625)

        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getString(), "Column 2")
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 5).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 6).getValue(), 0.5)
        self.assertEqual(get_cell_by_position(document, 0, 6, 7).getValue(), 0.25)
        self.assertEqual(get_cell_by_position(document, 0, 6, 8).getValue(), 0.125)
        self.assertEqual(get_cell_by_position(document, 0, 6, 9).getValue(), 0.0625)
        self.assertEqual(get_cell_by_position(document, 0, 6, 10).getValue(), 0.03125)
        self.assertEqual(get_cell_by_position(document, 0, 6, 11).getValue(), 0.015625)
        self.assertEqual(get_cell_by_position(document, 0, 6, 12).getValue(), 0.0078125)
        self.assertEqual(get_cell_by_position(document, 0, 6, 13).getValue(), 0.00390625)
        self.assertEqual(get_cell_by_position(document, 0, 6, 14).getValue(), 0.001953125)
        self.assertEqual(get_cell_by_position(document, 0, 6, 15).getValue(), 0.0009765625)
        self.assertEqual(get_cell_by_position(document, 0, 6, 16).getValue(), 0.00048828125)

        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:ExponentialSmoothingDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_exponential_smoothing_row(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #fill data
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "0")
        enter_text_to_cell(gridwin, "A3", "0")
        enter_text_to_cell(gridwin, "A4", "0")
        enter_text_to_cell(gridwin, "A5", "0")
        enter_text_to_cell(gridwin, "A7", "0")
        enter_text_to_cell(gridwin, "A8", "0")
        enter_text_to_cell(gridwin, "A9", "0")
        enter_text_to_cell(gridwin, "A10", "0")
        enter_text_to_cell(gridwin, "A11", "0")
        enter_text_to_cell(gridwin, "A12", "0")
        enter_text_to_cell(gridwin, "A13", "0")

        enter_text_to_cell(gridwin, "B1", "0")
        enter_text_to_cell(gridwin, "B2", "0")
        enter_text_to_cell(gridwin, "B3", "1")
        enter_text_to_cell(gridwin, "B4", "0")
        enter_text_to_cell(gridwin, "B5", "0")
        enter_text_to_cell(gridwin, "B6", "0")
        enter_text_to_cell(gridwin, "B7", "0")
        enter_text_to_cell(gridwin, "B8", "0")
        enter_text_to_cell(gridwin, "B9", "0")
        enter_text_to_cell(gridwin, "B10", "0")
        enter_text_to_cell(gridwin, "B11", "0")
        enter_text_to_cell(gridwin, "B12", "0")
        enter_text_to_cell(gridwin, "B13", "0")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B13"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:ExponentialSmoothingDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xinputrangeedit = xDialog.getChild("input-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")
        xmoothingfactorspin = xDialog.getChild("smoothing-factor-spin")

        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$B$13"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$F$1"}))
        xmoothingfactorspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xmoothingfactorspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xmoothingfactorspin.executeAction("TYPE", mkPropertyValues({"TEXT":"0.5"}))
        xgroupedbyrowsradio.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "Alpha")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue(), 0.5)
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getString(), "Row 1")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getValue(), 0.5)

        self.assertEqual(get_cell_by_position(document, 0, 6, 2).getString(), "Row 2")
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 6, 5).getValue(), 0)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
