# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 117367 - FORMATTING Merge cells dialog radio buttons cannot be unselected

class tdf117367(UITestCase):
    def test_tdf117367_merge_cells_radio_buttons(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "AAA")
        enter_text_to_cell(gridwin, "A2", "BBB")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B4"}))

        #Right-click - Merge cells  /Select all three options
        self.ui_test.execute_dialog_through_command(".uno:MergeCells")
        xDialog = self.xUITest.getTopFocusWindow()
        xmoveCellsRadio = xDialog.getChild("move-cells-radio")
        xemptyCellsRadio = xDialog.getChild("empty-cells-radio")
        xkeepContentRadio = xDialog.getChild("keep-content-radio")
        xmoveCellsRadio.executeAction("CLICK", tuple())
        xmoveCellsRadio.executeAction("CLICK", tuple())
        #Issue: I think these should be exclusive, not possible to select more than one. It is also impossible to uncheck any of the buttons.
        self.assertEqual(get_state_as_dict(xmoveCellsRadio)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xemptyCellsRadio)["Checked"], "false")
        self.assertEqual(get_state_as_dict(xkeepContentRadio)["Checked"], "false")

        xemptyCellsRadio.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(xmoveCellsRadio)["Checked"], "false")
        self.assertEqual(get_state_as_dict(xemptyCellsRadio)["Checked"], "true")
        self.assertEqual(get_state_as_dict(xkeepContentRadio)["Checked"], "false")

        xkeepContentRadio.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(xmoveCellsRadio)["Checked"], "false")
        self.assertEqual(get_state_as_dict(xemptyCellsRadio)["Checked"], "false")
        self.assertEqual(get_state_as_dict(xkeepContentRadio)["Checked"], "true")

        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
