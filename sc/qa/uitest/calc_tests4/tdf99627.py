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

#Bug 99627 - Calc freezes when applying natural sorting on text columns

class tdf99627(UITestCase):
    def test_td99627_natural_sort(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf99627.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        #Open sort dialog by DATA - SORT
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xNatural = xDialog.getChild("naturalsort")
        xdown = xDialog.getChild("down")
        xNatural.executeAction("CLICK", tuple())
        select_pos(xTabs, "0")
        xdown.executeAction("CLICK", tuple())
        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "2998")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2998).getString(), "1")
        #UNDO
        self.xUITest.executeCommand(".uno:Undo")
        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "1")
        self.assertEqual(get_cell_by_position(document, 0, 0, 2998).getString(), "2998")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: