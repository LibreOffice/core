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
from uitest.debug import sleep
#Bug 53920 - EDITING: Data Validity Cell Range not being applied to multiple selected cells
class tdf53920(UITestCase):
    def test_tdf53920_validity_multiple_cells(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #. Provide the desired values in a range of cells
        enter_text_to_cell(gridwin, "C1", "A")
        enter_text_to_cell(gridwin, "C2", "B")
        enter_text_to_cell(gridwin, "C3", "C")
        #Select the cells to be validated
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3", "EXTEND":"1"}))
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A5", "EXTEND":"1"}))
        #Apply Data > Validity ... > Cell Range
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xmin = xDialog.getChild("min")

        props = {"TEXT": "Cell range"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"$Sheet1.$C$1:$C$3"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #Expected behavior: All selected cells validate data.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xmin = xDialog.getChild("min")
        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Cell range")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "$Sheet1.$C$1:$C$3")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xmin = xDialog.getChild("min")
        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Cell range")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "$Sheet1.$C$1:$C$3")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A5"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xmin = xDialog.getChild("min")
        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Cell range")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "$Sheet1.$C$1:$C$3")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
