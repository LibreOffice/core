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

class descriptiveStatistics(UITestCase):
    def test_descriptive_statistics(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #fill data
        enter_text_to_cell(gridwin, "A1", "Math")
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
        self.ui_test.execute_modeless_dialog_through_command(".uno:DescriptiveStatisticsDialog")
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
        xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$F$1"}))

        xgroupedbyrowsradio.executeAction("CLICK", tuple())
        xgroupedbycolumnsradio.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 5, 1).getString() , "Mean")
        self.assertEqual(get_cell_by_position(document, 0, 5, 2).getString() , "Standard Error")
        self.assertEqual(get_cell_by_position(document, 0, 5, 3).getString() , "Mode")
        self.assertEqual(get_cell_by_position(document, 0, 5, 4).getString() , "Median")
        self.assertEqual(get_cell_by_position(document, 0, 5, 5).getString() , "First Quartile")
        self.assertEqual(get_cell_by_position(document, 0, 5, 6).getString() , "Third Quartile")
        self.assertEqual(get_cell_by_position(document, 0, 5, 7).getString() , "Variance")
        self.assertEqual(get_cell_by_position(document, 0, 5, 8).getString() , "Standard Deviation")
        self.assertEqual(get_cell_by_position(document, 0, 5, 9).getString() , "Kurtosis")
        self.assertEqual(get_cell_by_position(document, 0, 5, 10).getString() , "Skewness")
        self.assertEqual(get_cell_by_position(document, 0, 5, 11).getString() , "Range")
        self.assertEqual(get_cell_by_position(document, 0, 5, 12).getString() , "Minimum")
        self.assertEqual(get_cell_by_position(document, 0, 5, 13).getString() , "Maximum")
        self.assertEqual(get_cell_by_position(document, 0, 5, 14).getString() , "Sum")
        self.assertEqual(get_cell_by_position(document, 0, 5, 15).getString() , "Count")

        self.assertEqual(get_cell_by_position(document, 0, 6, 0).getString() , "Column 1")
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 1).getValue(),12) , 41.909090909091)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 2).getValue(),13) , 3.5610380137731)
        self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue() , 47)
        self.assertEqual(get_cell_by_position(document, 0, 6, 4).getValue() , 40)
        self.assertEqual(get_cell_by_position(document, 0, 6, 5).getValue() , 32.5)
        self.assertEqual(get_cell_by_position(document, 0, 6, 6).getValue() , 51.5)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 7).getValue(),11) , 139.49090909091)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 8).getValue(),12) , 11.810626955878)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 9).getValue(),13) , -1.4621677980825)
        self.assertEqual(round(get_cell_by_position(document, 0, 6, 10).getValue(),14) , 0.01524095329036)
        self.assertEqual(get_cell_by_position(document, 0, 6, 11).getValue() , 31)
        self.assertEqual(get_cell_by_position(document, 0, 6, 12).getValue() , 26)
        self.assertEqual(get_cell_by_position(document, 0, 6, 13).getValue() , 57)
        self.assertEqual(get_cell_by_position(document, 0, 6, 14).getValue() , 461)
        self.assertEqual(get_cell_by_position(document, 0, 6, 15).getValue() , 11)

        self.assertEqual(get_cell_by_position(document, 0, 7, 0).getString() , "Column 2")
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 1).getValue(),1) , 59.7)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 2).getValue(),14) , 5.35837869343164)
        self.assertEqual(get_cell_by_position(document, 0, 7, 3).getValue() , 49)
        self.assertEqual(get_cell_by_position(document, 0, 7, 4).getValue() , 64.5)
        self.assertEqual(get_cell_by_position(document, 0, 7, 5).getValue() , 49)
        self.assertEqual(get_cell_by_position(document, 0, 7, 6).getValue() , 67.75)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 7).getValue(),11) , 287.12222222222)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 8).getValue(),12) , 16.944681236961)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 9).getValue(),14) , -0.94159887458526)
        self.assertEqual(round(get_cell_by_position(document, 0, 7, 10).getValue(),13) , -0.2226426904338)
        self.assertEqual(get_cell_by_position(document, 0, 7, 11).getValue() , 51)
        self.assertEqual(get_cell_by_position(document, 0, 7, 12).getValue() , 33)
        self.assertEqual(get_cell_by_position(document, 0, 7, 13).getValue() , 84)
        self.assertEqual(get_cell_by_position(document, 0, 7, 14).getValue() , 597)
        self.assertEqual(get_cell_by_position(document, 0, 7, 15).getValue() , 10)

        self.assertEqual(get_cell_by_position(document, 0, 8, 0).getString() , "Column 3")
        self.assertEqual(get_cell_by_position(document, 0, 8, 1).getValue() , 44.7)
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 2).getValue(),13) , 4.7680650629416)
        self.assertEqual(get_cell_by_position(document, 0, 8, 3).getValue() , 60)
        self.assertEqual(get_cell_by_position(document, 0, 8, 4).getValue() , 43.5)
        self.assertEqual(get_cell_by_position(document, 0, 8, 5).getValue() , 40.5)
        self.assertEqual(get_cell_by_position(document, 0, 8, 6).getValue() , 57.75)
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 7).getValue(),11) , 227.34444444444)
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 8).getValue(),11) , 15.07794563077)
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 9).getValue(),12) , 1.418052718986)
        self.assertEqual(round(get_cell_by_position(document, 0, 8, 10).getValue(),14) , -0.97668033725691)
        self.assertEqual(get_cell_by_position(document, 0, 8, 11).getValue() , 50)
        self.assertEqual(get_cell_by_position(document, 0, 8, 12).getValue() , 12)
        self.assertEqual(get_cell_by_position(document, 0, 8, 13).getValue() , 62)
        self.assertEqual(get_cell_by_position(document, 0, 8, 14).getValue() , 447)
        self.assertEqual(get_cell_by_position(document, 0, 8, 15).getValue() , 10)
        #undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 5, 0).getString(), "")

        # test cancel button
        self.ui_test.execute_modeless_dialog_through_command(".uno:DescriptiveStatisticsDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
