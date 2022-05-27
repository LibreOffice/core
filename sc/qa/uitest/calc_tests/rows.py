# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import change_measurement_unit
from libreoffice.uno.propertyvalue import mkPropertyValues

class CalcRows(UITestCase):
    def test_row_height(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #row height
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                xdefault = xDialog.getChild("default")
                self.assertEqual(get_state_as_dict(xdefault)["Selected"], "true")  #default selected

                # tdf#144247: Without the fix in place, this test would have failed with
                # AssertionError: '0.45 cm' != '0.4516 cm'
                self.assertEqual("0.45 cm", get_state_as_dict(xvalue)["Text"])
                xvalue.executeAction("UP", tuple())
                self.assertEqual("0.50 cm", get_state_as_dict(xvalue)["Text"])

                self.assertEqual(get_state_as_dict(xdefault)["Selected"], "false")  #default not selected
                xdefault.executeAction("CLICK", tuple())  #click default
                self.assertEqual("0.45 cm", get_state_as_dict(xvalue)["Text"])

                #write your own value
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #verify
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight", close_button="cancel") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


    def test_row_height_two_rows(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3", "EXTEND":"1"}))

            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                #write your own value
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


    def test_tdf89140_row_height_copy(self):
        #Bug 89140 - Calc row paste doesn't keep row height
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #row height
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #select row 1
            self.xUITest.executeCommand(".uno:SelectRow")
            #copy
            self.xUITest.executeCommand(".uno:Copy")
            #select A3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            #paste
            self.xUITest.executeCommand(".uno:Paste")
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


    def test_row_hide_show(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select A3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            self.xUITest.executeCommand(".uno:HideRow") #uno command moves focus one cell down
            #verify A4
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"UP"}))
            #verify A2 (row 3 is hidden)
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "1")
            #Show hidden row: select A2:A4
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A4"}))
            self.xUITest.executeCommand(".uno:ShowRow")
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A4"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"UP"}))
            #verify A3 (row 3 is not hidden)
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "2")


    def test_row_test_move(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select A3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "2")
            #down
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"DOWN"}))
            #verify A4
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"UP"}))
            #verify A2
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "2")


    def test_row_height_insert_below(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select A3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            #row height
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #select row 3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            self.xUITest.executeCommand(".uno:SelectRow")
            #insert rows below
            self.xUITest.executeCommand(".uno:InsertRowsAfter")

            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A4"}))
            with self.ui_test.execute_dialog_through_command(".uno:RowHeight") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
