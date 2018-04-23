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
#Bug 99773 - EDITING: calc freezes if sorting in natural mode

class tdf99773(UITestCase):
    def test_tdf99773_natural_sorting_space(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #enter data
        enter_text_to_cell(gridwin, "A1", "A 11")
        enter_text_to_cell(gridwin, "A2", "A 2")
        enter_text_to_cell(gridwin, "A3", "B 2")
        enter_text_to_cell(gridwin, "A4", "A 5")
        enter_text_to_cell(gridwin, "A5", "A 50")
        enter_text_to_cell(gridwin, "A6", "B 20")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A6"}))
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
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "A 2")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "A 5")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "A 11")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "A 50")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "B 2")
        self.assertEqual(get_cell_by_position(document, 0, 0, 5).getString(), "B 20")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
