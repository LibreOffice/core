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
#Testcases Sorting TCS_Sorting

class CalcNaturalSorting(UITestCase):

    def test_natural_sorting_rows(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #enter data
        enter_text_to_cell(gridwin, "A1", "MW100SSMOU456.996JIL4")
        enter_text_to_cell(gridwin, "A2", "MW180SSMOU456.996JIL4")
        enter_text_to_cell(gridwin, "A3", "MW110SSMOU456.993JIL4")
        enter_text_to_cell(gridwin, "A4", "MW180SSMOU456.994JIL4")
        enter_text_to_cell(gridwin, "A5", "MW101SSMOU456.996JIL4")
        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xNatural = xDialog.getChild("naturalsort")
        xNatural.executeAction("CLICK", tuple())
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "MW100SSMOU456.996JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "MW101SSMOU456.996JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "MW110SSMOU456.993JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "MW180SSMOU456.994JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "MW180SSMOU456.996JIL4")
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "MW100SSMOU456.996JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "MW180SSMOU456.996JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "MW110SSMOU456.993JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "MW180SSMOU456.994JIL4")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "MW101SSMOU456.996JIL4")
        #enter data
        enter_text_to_cell(gridwin, "D1", "MW-2")
        enter_text_to_cell(gridwin, "D2", "MW-20")
        enter_text_to_cell(gridwin, "D3", "MW-1")
        enter_text_to_cell(gridwin, "D4", "MW-18")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "D1:D4"}))

        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xNatural = xDialog.getChild("naturalsort")
        if (get_state_as_dict(xNatural)["Selected"]) == "false":
            xNatural.executeAction("CLICK", tuple())
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "MW-1")
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getString(), "MW-2")
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getString(), "MW-18")
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getString(), "MW-20")

        self.ui_test.close_doc()

    def test_natural_sorting_columns(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #enter data
        enter_text_to_cell(gridwin, "A1", "MW-2")
        enter_text_to_cell(gridwin, "B1", "MW-20")
        enter_text_to_cell(gridwin, "C1", "MW-1")
        enter_text_to_cell(gridwin, "D1", "MW-18")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:D1"}))

        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        xleftright = xDialog.getChild("leftright")
        select_pos(xTabs, "1")
        xNatural = xDialog.getChild("naturalsort")
        xleftright.executeAction("CLICK", tuple())
        if (get_state_as_dict(xNatural)["Selected"]) == "false":
            xNatural.executeAction("CLICK", tuple())
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "MW-1")
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "MW-2")
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "MW-18")
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "MW-20")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
