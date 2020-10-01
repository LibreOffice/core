# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import change_measurement_unit
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.debug import sleep

class CalcColumns(UITestCase):
    def test_column_width(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #column width
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xdefault = xDialog.getChild("default")
        self.assertEqual(get_state_as_dict(xdefault)["Selected"], "true")  #default selected
        heightStrOrig = get_state_as_dict(xvalue)["Text"]
        heightVal = heightStrOrig[:4]  #default 2.26 cm
        xvalue.executeAction("UP", tuple())  #2.36 cm
        heightStr = get_state_as_dict(xvalue)["Text"]
        heightValNew = heightStr[:4]
        self.assertEqual(get_state_as_dict(xdefault)["Selected"], "false")  #default not selected
        self.assertEqual(heightValNew > heightVal, True)  #new value is bigger
        xdefault.executeAction("CLICK", tuple())  #click default
        self.assertEqual(get_state_as_dict(xvalue)["Text"] == heightStrOrig, True)  #default value set
        #write your own value
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")
        xCancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancel)

        self.ui_test.close_doc()

    def test_column_width_two_columns(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1", "EXTEND":"1"}))

        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xdefault = xDialog.getChild("default")
        #write your own value
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

    def test_column_width_copy(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #column width
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
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
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

    def test_column_hide_show(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
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

        self.ui_test.close_doc()

    def test_column_test_move(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
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

        self.ui_test.close_doc()

    def test_tdf117522_column_width_insert_left(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select C1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        #column width
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #select D1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        #column width
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"2 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #select E1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
        #column width
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"3 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #select columns C-E
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:E1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        #Insert Columns Left
        self.xUITest.executeCommand(".uno:InsertColumnsBefore")
        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "C1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "2.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "E1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "3.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "F1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "G1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "2.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "H1"}))
        self.ui_test.execute_dialog_through_command(".uno:ColumnWidth")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "3.00 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
