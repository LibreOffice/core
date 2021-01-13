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

#Bug 118206 - [GTK3] Calc hangs copying/cutting a conditional format column
class tdf118206(UITestCase):
    def test_tdf118206(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf118206.xlsx"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        self.xUITest.executeCommand(".uno:Copy")
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        self.xUITest.executeCommand(".uno:Paste")

        #verify
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "On Back Order")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 1, 7).getValue(), 1)

        #Undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "On Back Order")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 0, 7).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getString(), "")
        self.assertEqual(get_cell_by_position(document, 0, 1, 7).getString(), "")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
