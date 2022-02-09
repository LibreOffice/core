# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Test for the AutoFill feature - auto-fill can't increment last octet of ip addresses

class CalcAutofill(UITestCase):

    def test_autofill(self):
        with self.ui_test.load_file(get_url_for_data_file("autofill.ods")) as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #Select cell A12 and drag the fill handle in the bottom right corner of the cell down to A18
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A12:A18"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries"):
                pass
            #Compare with the content in the right next column
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 11).getValue(), 18.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 12).getValue(), 19.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 13).getValue(), 20.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 14).getValue(), 21.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 15).getValue(), 22.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 16).getValue(), 23.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 17).getValue(), 24.34)
            #Select cell A12 and drag the fill handle in the bottom right corner of the cell up to A6
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A6:A12"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xup = xDialog.getChild("up")
                xincrement = xDialog.getChild("increment")
                xup.executeAction("CLICK", tuple())
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"TEXT":"-1"}))
            #Compare with the content in the right next column
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 5).getValue(), 12.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 6).getValue(), 13.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 7).getValue(), 14.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 8).getValue(), 15.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 9).getValue(), 16.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 10).getValue(), 17.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 11).getValue(), 18.34)

            #Test that hidden cells are not affected / skipped in the increment process.
            #Simulate selecting cell A26 and dragging the fill handle in the bottom right corner of the cell down to A32
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A26:A32"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries"):
                pass
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 25).getValue(), 18.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 26).getValue(), 19.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 27).getValue(), 5.0) #hidden
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 28).getValue(), 5.0) #hidden
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 29).getString(), "hiddenA30")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 30).getValue(), 20.34) #overwrite "rows"
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 31).getValue(), 21.34)
            #Simulate selecting cell A26 and dragging the fill handle in the bottom right corner of the cell up to A19
            #   Note: start at empty cell A19 so Sheet - Fill Cells - Fill Series has good defaults
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A19:A26"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xup = xDialog.getChild("up")
                xincrement = xDialog.getChild("increment")
                xup.executeAction("CLICK", tuple())
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"TEXT":"-1"}))
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 19).getString(), "hiddenA20")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 20).getValue(), 15.34)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 21).getValue(), 5.0) #hidden
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 22).getValue(), 16.34) #overwrite "testing"
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 23).getValue(), 5.0) #hidden
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 24).getValue(), 17.34) #overwrite "hidden"
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 25).getValue(), 18.34)

            #Continue with the next cells with grey background
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "M12:M18"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries"):
                pass
            #Compare with the content in the right next column
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 11).getString(), "12abc40")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 12).getString(), "12abc41")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 13).getString(), "12abc42")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 14).getString(), "12abc43")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 15).getString(), "12abc44")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 16).getString(), "12abc45")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 17).getString(), "12abc46")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "M6:M12"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xup = xDialog.getChild("up")
                xincrement = xDialog.getChild("increment")
                xup.executeAction("CLICK", tuple())
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"TEXT":"-1"}))
            #Compare with the content in the right next column
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 5).getString(), "12abc34")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 6).getString(), "12abc35")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 7).getString(), "12abc36")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 8).getString(), "12abc37")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 9).getString(), "12abc38")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 10).getString(), "12abc39")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 12, 11).getString(), "12abc40")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "P12:P18"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries"):
                pass
            #Compare with the content in the right next column
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 11).getString(), "10.64.127.7")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 12).getString(), "10.64.127.8")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 13).getString(), "10.64.127.9")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 14).getString(), "10.64.127.10")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 15).getString(), "10.64.127.11")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 16).getString(), "10.64.127.12")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 17).getString(), "10.64.127.13")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "P6:P12"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xup = xDialog.getChild("up")
                xincrement = xDialog.getChild("increment")
                xup.executeAction("CLICK", tuple())
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xincrement.executeAction("TYPE", mkPropertyValues({"TEXT":"-1"}))
            #Compare with the content in the right next column
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 5).getString(), "10.64.127.1")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 6).getString(), "10.64.127.2")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 7).getString(), "10.64.127.3")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 8).getString(), "10.64.127.4")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 9).getString(), "10.64.127.5")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 10).getString(), "10.64.127.6")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 15, 11).getString(), "10.64.127.7")

    def test_autofill_with_suffix(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            enter_text_to_cell(gridwin, "A1", "1st")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A3"}))
            with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
                xautofill = xDialog.getChild("autofill")
                xautofill.executeAction("CLICK", tuple())
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "1st")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "2nd")
            self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "3rd")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
