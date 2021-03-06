# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#tdf#89754 - EDITING: 'Autofill - Date' don't increment non different consecutive date cells

class tdf89754(UITestCase):

    def test_tdf89754_autofill_date(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #1 - A1: 2012-10-31 and A2: 2012-10-31
        enter_text_to_cell(gridwin, "A1", "2012-10-31")
        enter_text_to_cell(gridwin, "A2", "2012-10-31")

        #2 - Select A1:A5
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A5"}))

        #3 - Edit -> fill -> Series -> Down
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        #4 - Expected: All Cells show the initial date
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getValue(), 41213)
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 41213)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 41213)
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 41213)
        self.assertEqual(get_cell_by_position(document, 0, 0, 4).getValue(), 41213)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
