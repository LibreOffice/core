# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 49531 - EDITING: Sort rows for will sort columns
#Bug 49520 - EDITING: CRASH when undo sort with chart

class tdf49531(UITestCase):
    def test_td49531_sort_undo_crash(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf49531.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select A3:C147
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A3:C147"}))

        #Menu 'Data -> Sort -> Top to bottom - all otheroptions unchecked
        #Column B - Ascending' <ok>
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xNatural = xDialog.getChild("naturalsort")
        xtopdown = xDialog.getChild("topdown")
        xHeader = xDialog.getChild("header")
        if (get_state_as_dict(xNatural)["Selected"]) == "true":
            xNatural.executeAction("CLICK", tuple())
        if (get_state_as_dict(xHeader)["Selected"]) == "true":
            xHeader.executeAction("CLICK", tuple())
        xtopdown.executeAction("CLICK", tuple())
        select_pos(xTabs, "0")
        xSortKey1 = xDialog.getChild("sortlb")
        xAsc = xDialog.getChild("up")
        select_by_text(xSortKey1, "Column B")
        xAsc.executeAction("CLICK", tuple())
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "x")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "0")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 111)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 48)
        # UNDO Bug 49520 - EDITING: CRASH when undo sort with chart
        self.xUITest.executeCommand(".uno:Undo")
        # Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "x")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "0")
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 3)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
