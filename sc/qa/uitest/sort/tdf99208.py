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

#Bug 99208 - Spreadsheet sort hangs

class tdf99208(UITestCase):
    def test_td99627_natural_sort(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf99208.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select A3:C245
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C245"}))

        #Menu 'Data -> Sort
        #Column A - Ascending' <ok>
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xNatural = xDialog.getChild("naturalsort")
        xtopdown = xDialog.getChild("topdown")
        xHeader = xDialog.getChild("header")
        xFormats = xDialog.getChild("formats")
        if (get_state_as_dict(xNatural)["Selected"]) == "false":
            xNatural.executeAction("CLICK", tuple())
        if (get_state_as_dict(xHeader)["Selected"]) == "false":
            xHeader.executeAction("CLICK", tuple())
        if (get_state_as_dict(xFormats)["Selected"]) == "false":
            xFormats.executeAction("CLICK", tuple())
        xtopdown.executeAction("CLICK", tuple())
        select_pos(xTabs, "0")
        xSortKey1 = xDialog.getChild("sortlb")
        xAsc = xDialog.getChild("up")
        props = {"TEXT": "FODMAP"}
        actionProps = mkPropertyValues(props)
        xSortKey1.executeAction("SELECT", actionProps)
        xAsc.executeAction("CLICK", tuple())
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #Verify Expected: Values column B sorted ascending, column "control" unsorted
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "FODMAP")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "agave")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "almond milk")
        self.assertEqual(get_cell_by_position(document, 0, 0, 244).getString(), "zucchini")
        # UNDO
        self.xUITest.executeCommand(".uno:Undo")
        # Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "FODMAP")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "beef (grass fed, no breadcrumbs)")
        self.assertEqual(get_cell_by_position(document, 0, 0, 244).getString(), "salsa")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: