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

class tdf45020(UITestCase):
    def test_tdf45020_hide_rows_select(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #in cell A1-A4: apple pear melon mango
        enter_text_to_cell(gridwin, "A1", "apple")
        enter_text_to_cell(gridwin, "A1", "pear")
        enter_text_to_cell(gridwin, "A1", "melon")
        enter_text_to_cell(gridwin, "A1", "mango")
        #select A2 and A3
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A3"}))
        #format > Row > Hide
        self.ui_test._xUITest.executeCommand(".uno:HideRow")
        #select A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #press Shift+down
        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE":"SHIFT+DOWN"}))
        #verify A1:A4 selected
        gridWinState = get_state_as_dict(gridwin)
        self.assertEqual(gridWinState["MarkedArea"], "Sheet1.A1:Sheet1.A4")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
