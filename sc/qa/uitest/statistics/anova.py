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

class anova(UITestCase):
    def test_statistic_anova(self):
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
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:C13"}))
        self.ui_test.execute_modeless_dialog_through_command(".uno:AnalysisOfVarianceDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xinputrangeedit = xDialog.getChild("input-range-edit")
        xoutputrangeedit = xDialog.getChild("output-range-edit")
        xradiotwofactor = xDialog.getChild("radio-two-factor")
        xradiosinglefactor = xDialog.getChild("radio-single-factor")
        xgroupedbyrowsradio = xDialog.getChild("groupedby-rows-radio")
        xgroupedbycolumnsradio = xDialog.getChild("groupedby-columns-radio")
        xalphaspin = xDialog.getChild("alpha-spin")

        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$2:$C$13"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$F$1"}))
        xradiosinglefactor.executeAction("CLICK", tuple())
        xalphaspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xalphaspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xalphaspin.executeAction("TYPE", mkPropertyValues({"TEXT":"0.05"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "ANOVA - Single Factor")
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getString(), "Alpha")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getString(), "Groups")
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getString(), "Column 1")
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getString(), "Column 2")
        self.assertEqual(get_cell_by_position(document, 0, 5, 6).getString(), "Column 3")
        self.assertEqual(get_cell_by_position(document, 0, 5, 8).getString(), "Source of Variation")
        self.assertEqual(get_cell_by_position(document, 0, 5, 9).getString(), "Between Groups")
        self.assertEqual(get_cell_by_position(document, 0, 5, 10).getString(), "Within Groups")
        self.assertEqual(get_cell_by_position(document, 0, 5, 11).getString(), "Total")

        self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue(), 0.05)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getString(), "Count")
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getValue(), 11)
        self.assertEqual(get_cell_by_position(document, 0, 6, 5).getValue(), 10)
        self.assertEqual(get_cell_by_position(document, 0, 6, 6).getValue(), 10)
        self.assertEqual(get_cell_by_position(document, 0, 6, 8).getString(), "SS")
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 9).getValue(),11), 1876.56832844575)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 10).getValue(),10), 6025.1090909091)
        #bug 80583
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 11).getValue(),11), 7901.67741935484)

        self.assertEqual(get_cell_by_position(document, 0, 7, 3).getString(), "Sum")
        self.assertEqual(get_cell_by_position(document, 0, 7, 4).getValue(), 461)
        self.assertEqual(get_cell_by_position(document, 0, 7, 5).getValue(), 597)
        self.assertEqual(get_cell_by_position(document, 0, 7, 6).getValue(), 447)
        self.assertEqual(get_cell_by_position(document, 0, 7, 8).getString(), "df")
        self.assertEqual(get_cell_by_position(document, 0, 7, 9).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 7, 10).getValue(), 28)
        self.assertEqual(get_cell_by_position(document, 0, 7, 11).getValue(), 30)

        self.assertEqual(get_cell_by_position(document, 0, 8, 3).getString(), "Mean")
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 4).getValue(), 12), 41.909090909091)
        self.assertEqual(get_cell_by_position(document, 0, 8, 5).getValue(), 59.7)
        self.assertEqual(get_cell_by_position(document, 0, 8, 6).getValue(), 44.7)
        self.assertEqual(get_cell_by_position(document, 0, 8, 8).getString(), "MS")
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 9).getValue(),11), 938.28416422287)
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 10).getValue(),11), 215.18246753247)

        self.assertEqual(get_cell_by_position(document, 0, 9, 3).getString(), "Variance")
        self.assertEqual(round(get_cell_by_position(document, 0, 9, 4).getValue(),11), 139.49090909091)
        self.assertEqual(round(get_cell_by_position(document, 0, 9, 5).getValue(),12), 287.122222222222)
        self.assertEqual(round(get_cell_by_position(document, 0, 9, 6).getValue(),12), 227.344444444444)
        self.assertEqual(get_cell_by_position(document, 0, 9, 8).getString(), "F")
        self.assertEqual(round(get_cell_by_position(document, 0, 9, 9).getValue(),13), 4.3604117704492)

        self.assertEqual(get_cell_by_position(document, 0, 10, 8).getString(), "P-value")
        self.assertEqual(round(get_cell_by_position(document, 0, 10, 9).getValue(),14), 0.02246149518798)

        self.assertEqual(get_cell_by_position(document, 0, 11, 8).getString(), "F critical")
        self.assertEqual(round(get_cell_by_position(document, 0, 11, 9).getValue(),13), 3.3403855582378)

        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:AnalysisOfVarianceDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
