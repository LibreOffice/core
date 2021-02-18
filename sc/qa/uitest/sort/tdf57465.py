# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf57465(UITestCase):

    def test_tdf57465(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf57465.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:G4"}))

        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")

        xHeader = xDialog.getChild("header")
        if (get_state_as_dict(xHeader)["Selected"]) == 'true':
            xHeader.executeAction("CLICK", tuple())

        xLeftRight = xDialog.getChild("leftright")
        xLeftRight.executeAction("CLICK", tuple())

        select_pos(xTabs, "0")

        self.assertEqual("Row 1", get_state_as_dict(xDialog.getChild("sortlb"))['DisplayText'])

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.assertEqual("a", get_cell_by_position(document, 0, 1, 1).getString())

        # Without the fix in place, this test would have failed with
        # AssertionError: 'b' != ''
        self.assertEqual("b", get_cell_by_position(document, 0, 2, 2).getString())
        self.assertEqual("c", get_cell_by_position(document, 0, 3, 3).getString())
        self.assertEqual("d", get_cell_by_position(document, 0, 4, 1).getString())
        self.assertEqual("e", get_cell_by_position(document, 0, 5, 2).getString())
        self.assertEqual("f", get_cell_by_position(document, 0, 6, 3).getString())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
