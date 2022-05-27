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

class CalcColumns(UITestCase):
    def test_column_width(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #column width
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                xdefault = xDialog.getChild("default")
                self.assertEqual(get_state_as_dict(xdefault)["Selected"], "true")  #default selected

                # tdf#144247: Without the fix in place, this test would have failed with
                # AssertionError: '2.26 cm' != '2.2578 cm'
                self.assertEqual("2.26 cm", get_state_as_dict(xvalue)["Text"])
                xvalue.executeAction("UP", tuple())
                self.assertEqual("2.30 cm", get_state_as_dict(xvalue)["Text"])
                self.assertEqual(get_state_as_dict(xdefault)["Selected"], "false")  #default not selected
                xdefault.executeAction("CLICK", tuple())  #click default
                self.assertEqual("2.26 cm", get_state_as_dict(xvalue)["Text"])

                #write your own value
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #verify
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth", close_button="cancel") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


    def test_column_width_two_columns(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1", "EXTEND":"1"}))

            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                #write your own value
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


    def test_column_width_copy(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #column width
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #select column 1
            self.xUITest.executeCommand(".uno:SelectColumn")
            #copy
            self.xUITest.executeCommand(".uno:Copy")
            #select C1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            #paste
            self.xUITest.executeCommand(".uno:Paste")
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")


    def test_column_hide_show(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select A3
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            self.xUITest.executeCommand(".uno:HideColumn") #uno command moves focus one cell down
            #verify D1
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "3")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"LEFT"}))
            #verify B (column C is hidden)
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "1")
            #Show hidden column: select B1:D1
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:D1"}))
            self.xUITest.executeCommand(".uno:ShowColumn")
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "3")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"LEFT"}))
            #verify C1 (COlumn C is not hidden)
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "2")


    def test_column_test_move(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select C1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "2")
            #right
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"RIGHT"}))
            #verify D1
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "3")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"LEFT"}))
            #verify C1
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentColumn"], "2")


    def test_tdf117522_column_width_insert_left(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            change_measurement_unit(self, "Centimeter")

            #select C1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            #column width
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
                # Click Ok
            #select D1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
            #column width
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"2 cm"}))
                # Click Ok
            #select E1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
            #column width
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"3 cm"}))
                # Click Ok
            #select columns C-E
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:E1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")
            #Insert Columns Left
            self.xUITest.executeCommand(".uno:InsertColumnsBefore")
            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "2.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "3.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "F1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "G1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "2.00 cm")

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "H1"}))
            with self.ui_test.execute_dialog_through_command(".uno:ColumnWidth") as xDialog:
                xvalue = xDialog.getChild("value")
                self.assertEqual(get_state_as_dict(xvalue)["Text"], "3.00 cm")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
