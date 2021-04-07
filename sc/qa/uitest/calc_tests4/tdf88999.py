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
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 88999 - UI: Scientific format: unable to modify number of decimal places through Sidebar or Format > Cells

class tdf88999(UITestCase):
    def test_tdf88999_scientific_format_decimal_value(self):
        #numberingformatpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "1e-2")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        #select cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #format - cell
        self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")  #tab Numbers
        xliststore1 = xDialog.getChild("categorylb")  #1st list / Category
        xliststore2 = xDialog.getChild("formatlb")  #2nd list / Format
        xdecimalsed = xDialog.getChild("decimalsed")
        xleadzerosed = xDialog.getChild("leadzerosed")
        xnegnumred = xDialog.getChild("negnumred")
        xthousands = xDialog.getChild("thousands")
        xlanguagelb = xDialog.getChild("languagelb")
        xformatted = xDialog.getChild("formatted")

        self.assertEqual(get_state_as_dict(xliststore1)["SelectEntryText"], "Scientific")
        self.assertEqual(get_state_as_dict(xdecimalsed)["Text"], "2")
        self.assertEqual(get_state_as_dict(xdecimalsed)["Enabled"], "true")
        self.assertEqual(get_state_as_dict(xleadzerosed)["Text"], "1")
        self.assertEqual(get_state_as_dict(xformatted)["Text"], "0.00E+00")
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
