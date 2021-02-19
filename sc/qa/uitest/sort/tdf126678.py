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

class tdf126678(UITestCase):

    def execute_sort_dialog(self, gridwin, bIncludeFormats):
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B3"}))

        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")

        xIncludeFormats = xDialog.getChild("formats")

        if (get_state_as_dict(xIncludeFormats)["Selected"]) != bIncludeFormats:
            xIncludeFormats.executeAction("CLICK", tuple())

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

    def test_tdf126678(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "Text 2")
        enter_text_to_cell(gridwin, "A2", "Text 3")
        enter_text_to_cell(gridwin, "A3", "Text 1")

        # Set the background of the corresponding cell
        colorProperty = mkPropertyValues({"BackgroundColor": 16776960})
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B2"}))
        self.xUITest.executeCommandWithParameters(".uno:BackgroundColor", colorProperty)

        self.execute_sort_dialog(gridwin, "false")

        self.assertEqual("Text 1", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual("Text 2", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual("Text 3", get_cell_by_position(document, 0, 0, 2).getString())

        # Sorting without option "including formats" does not include cells with cell formats
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).CellBackColor, -1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).CellBackColor, 16776960)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).CellBackColor, -1)

        self.xUITest.executeCommand(".uno:Undo")

        self.execute_sort_dialog(gridwin, "true")

        self.assertEqual("Text 1", get_cell_by_position(document, 0, 0, 0).getString())
        self.assertEqual("Text 2", get_cell_by_position(document, 0, 0, 1).getString())
        self.assertEqual("Text 3", get_cell_by_position(document, 0, 0, 2).getString())

        # Sorting with option "including formats" includes all cells with visible cell formats
        # tdf126678 - Without the fix in place, the test would have failed with
        # AssertionError: -1 != 16776960
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).CellBackColor, -1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).CellBackColor, -1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).CellBackColor, 16776960)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
