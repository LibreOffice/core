# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #row height
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xdefault = xDialog.getChild("default")
        self.assertEqual(get_state_as_dict(xdefault)["Selected"], "true")  #default selected
        heightStrOrig = get_state_as_dict(xvalue)["Text"]
        heightVal = heightStrOrig[:4]  #default 0.45 cm
        xvalue.executeAction("UP", tuple())  #0.50 cm
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
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.0001 cm")
        xCancel = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancel)

        self.ui_test.close_doc()

    def test_row_height_two_rows(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3", "EXTEND":"1"}))

        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
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
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.0001 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.0001 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

    def test_tdf89140_row_height_copy(self):
        #Bug 89140 - Calc row paste doesn't keep row height
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #row height
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
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
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.0001 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

    def test_row_hide_show(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
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

        self.ui_test.close_doc()

    def test_row_test_move(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
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

        self.ui_test.close_doc()

    def test_row_height_insert_below(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        change_measurement_unit(self, "Centimeter")

        #select A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #row height
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #select row 3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        self.xUITest.executeCommand(".uno:SelectRow")
        #insert rows below
        self.xUITest.executeCommand(".uno:InsertRowsAfter")

        #verify
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.0001 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A4"}))
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        self.assertEqual(get_state_as_dict(xvalue)["Text"], "1.0001 cm")
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
