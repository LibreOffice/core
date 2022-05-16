# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class sampling(UITestCase):
    def test_statistic_sampling(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #fill data
            enter_text_to_cell(gridwin, "A1", "11")
            enter_text_to_cell(gridwin, "A2", "12")
            enter_text_to_cell(gridwin, "A3", "13")
            enter_text_to_cell(gridwin, "A4", "14")
            enter_text_to_cell(gridwin, "A5", "15")
            enter_text_to_cell(gridwin, "A6", "16")
            enter_text_to_cell(gridwin, "A7", "17")
            enter_text_to_cell(gridwin, "A8", "18")
            enter_text_to_cell(gridwin, "A9", "19")

            enter_text_to_cell(gridwin, "B1", "21")
            enter_text_to_cell(gridwin, "B2", "22")
            enter_text_to_cell(gridwin, "B3", "23")
            enter_text_to_cell(gridwin, "B4", "24")
            enter_text_to_cell(gridwin, "B5", "25")
            enter_text_to_cell(gridwin, "B6", "26")
            enter_text_to_cell(gridwin, "B7", "27")
            enter_text_to_cell(gridwin, "B8", "28")
            enter_text_to_cell(gridwin, "B9", "29")

            enter_text_to_cell(gridwin, "C1", "31")
            enter_text_to_cell(gridwin, "C2", "32")
            enter_text_to_cell(gridwin, "C3", "33")
            enter_text_to_cell(gridwin, "C4", "34")
            enter_text_to_cell(gridwin, "C5", "35")
            enter_text_to_cell(gridwin, "C6", "36")
            enter_text_to_cell(gridwin, "C7", "37")
            enter_text_to_cell(gridwin, "C8", "38")
            enter_text_to_cell(gridwin, "C9", "39")
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C9"}))
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SamplingDialog") as xDialog:
                xinputrangeedit = xDialog.getChild("input-range-edit")
                xoutputrangeedit = xDialog.getChild("output-range-edit")
                xrandommethodradio = xDialog.getChild("random-method-radio")
                xsamplesizespin = xDialog.getChild("sample-size-spin")
                xperiodicmethodradio = xDialog.getChild("periodic-method-radio")
                xperiodspin = xDialog.getChild("period-spin")

                xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xinputrangeedit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xinputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$A$1:$C$9"}))

                xoutputrangeedit.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$F$1"}))

                xperiodicmethodradio.executeAction("CLICK", tuple())

                xperiodspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xperiodspin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xperiodspin.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
            #Verify
            self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 12)
            self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue() , 14)
            self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue() , 16)
            self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue() , 18)

            self.assertEqual(get_cell_by_position(document, 0, 6, 0).getValue(), 22)
            self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue() , 24)
            self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue() , 26)
            self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue() , 28)

            self.assertEqual(get_cell_by_position(document, 0, 7, 0).getValue(), 32)
            self.assertEqual(get_cell_by_position(document, 0, 7, 1).getValue() , 34)
            self.assertEqual(get_cell_by_position(document, 0, 7, 2).getValue() , 36)
            self.assertEqual(get_cell_by_position(document, 0, 7, 3).getValue() , 38)
            #undo
            self.xUITest.executeCommand(".uno:Undo")
            self.assertEqual(get_cell_by_position(document, 0, 5, 0).getValue(), 0)
            self.assertEqual(get_cell_by_position(document, 0, 5, 1).getValue() , 0)
            self.assertEqual(get_cell_by_position(document, 0, 5, 2).getValue() , 0)
            self.assertEqual(get_cell_by_position(document, 0, 5, 3).getValue() , 0)

            self.assertEqual(get_cell_by_position(document, 0, 6, 0).getValue(), 0)
            self.assertEqual(get_cell_by_position(document, 0, 6, 1).getValue() , 0)
            self.assertEqual(get_cell_by_position(document, 0, 6, 2).getValue() , 0)
            self.assertEqual(get_cell_by_position(document, 0, 6, 3).getValue() , 0)

            self.assertEqual(get_cell_by_position(document, 0, 7, 0).getValue(), 0)
            self.assertEqual(get_cell_by_position(document, 0, 7, 1).getValue() , 0)
            self.assertEqual(get_cell_by_position(document, 0, 7, 2).getValue() , 0)
            self.assertEqual(get_cell_by_position(document, 0, 7, 3).getValue() , 0)

            # test cancel button
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SamplingDialog", close_button="cancel"):
                pass

    def test_tdf142986(self):
        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #fill data
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A200"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xStartValue = xDialog.getChild("startValue")
                xStartValue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xStartValue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xStartValue.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))

                xIncrement = xDialog.getChild("increment")
                xIncrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xIncrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xIncrement.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 199).getValue(), 200)

            with self.ui_test.execute_modeless_dialog_through_command(".uno:SamplingDialog") as xDialog:
                xInputRangeEdit = xDialog.getChild("input-range-edit")
                xOutputRangeEdit = xDialog.getChild("output-range-edit")
                xRandomMethodRadio = xDialog.getChild("random-method-radio")
                xSampleSizeSpin = xDialog.getChild("sample-size-spin")

                self.assertEqual("$Sheet1.$A$1:$A$200", get_state_as_dict(xInputRangeEdit)['Text'])

                xOutputRangeEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xOutputRangeEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xOutputRangeEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"$B$1"}))

                xRandomMethodRadio.executeAction("CLICK", tuple())

                xSampleSizeSpin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xSampleSizeSpin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xSampleSizeSpin.executeAction("TYPE", mkPropertyValues({"TEXT":"200"}))

            for i in range(200):
                self.assertTrue(get_cell_by_position(calc_doc, 0, 1, i).getValue() != 0.0,
                        "Value in cell B" + str(i) + " shouldn't be equal to 0.0")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
