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
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class DetectiveCircle(UITestCase):

    def test_delete_circle_at_formula(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "3")
        enter_text_to_cell(gridwin, "A3", "=SUM(A1:A2)")

        #Select the cells to be validated
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #Apply Data > Validity ... > Whole Numbers
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xdata = xDialog.getChild("data")
        xmin = xDialog.getChild("min")
        xmax = xDialog.getChild("max")

        props = {"TEXT": "Whole Numbers"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        xallowempty.executeAction("CLICK", tuple())
        propsA = {"TEXT": "equal"}
        actionPropsA = mkPropertyValues(propsA)
        xdata.executeAction("SELECT", actionPropsA)
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"5"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.xUITest.executeCommand(".uno:ShowInvalid")

        detectiveCircle1 = document.Sheets.getByName("Sheet1").DrawPage.getCount()
        #There should be 1 detective circle object!
        self.assertEqual(detectiveCircle1, 1)

        enter_text_to_cell(gridwin, "A1", "2")

        detectiveCircle2 = document.Sheets.getByName("Sheet1").DrawPage.getCount()
        #There should not be a detective circle object!
        self.assertEqual(detectiveCircle2, 0)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
