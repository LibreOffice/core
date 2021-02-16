# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 53482 - UI: Option 'Range contains column headings' ignored

class tdf53482(UITestCase):

    def test_tdf53482_Range_contains_column_headings_file(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf53482.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #1. Highlight cells to be sorted A8:J124
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A8:J124"}))
        #2. Click Data menu, Sort
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        #3. On Options tab, tick 'Range contains column labels'
        xHeader = xDialog.getChild("header")
        xHeader.executeAction("CLICK", tuple())
        if (get_state_as_dict(xHeader)["Selected"]) == "false":
            xHeader.executeAction("CLICK", tuple())
        #4. On Sort Criteria tab, set appropriate criteria
        select_pos(xTabs, "0")
        xDown = xDialog.getChild("down")
        xDown.executeAction("CLICK", tuple())
        xSortKey1 = xDialog.getChild("sortlb")
        props = {"TEXT": "Occupation"}
        actionProps = mkPropertyValues(props)
        xSortKey1.executeAction("SELECT", actionProps)
        #5. Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #6. Expected behavior:  Ignore column labels when sorting
        self.assertEqual(get_cell_by_position(document, 0, 6, 7).getString(), "Occupation")
        self.assertEqual(get_cell_by_position(document, 0, 6, 8).getString(), "Travel Industry")
        self.assertEqual(get_cell_by_position(document, 0, 6, 123).getString(), "13")

        self.ui_test.close_doc()

    def test_tdf53482_Range_contains_column_headings(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter: Misc; s; d; f; g
        enter_text_to_cell(gridwin, "A1", "Misc")
        enter_text_to_cell(gridwin, "A2", "s")
        enter_text_to_cell(gridwin, "A3", "d")
        enter_text_to_cell(gridwin, "A4", "f")
        enter_text_to_cell(gridwin, "A5", "g")
        #1. Highlight cells to be sorted
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))
        #2. Click Data menu, Sort
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        #3. On Options tab, tick 'Range contains column labels'
        xHeader = xDialog.getChild("header")
        xHeader.executeAction("CLICK", tuple())
        if (get_state_as_dict(xHeader)["Selected"]) == "false":
            xHeader.executeAction("CLICK", tuple())
        #4. On Sort Criteria tab, set appropriate criteria
        select_pos(xTabs, "0")
        xDown = xDialog.getChild("down")
        xDown.executeAction("CLICK", tuple())
        #5. Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        #6. Expected behavior:  Ignore column labels when sorting
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "Misc")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "s")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "g")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getString(), "f")
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getString(), "d")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: