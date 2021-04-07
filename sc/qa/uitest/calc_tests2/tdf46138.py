# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 46138 - Calc Frozen after "undo" "Filling Series Number in a Column"

class tdf46138(UITestCase):
    def test_tdf46138(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #Choose a column,
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        #select 'edit'>'fill'>'series'>Starting Value '1' then "OK",
        self.ui_test.execute_dialog_through_command(".uno:FillSeries")
        xDialog = self.xUITest.getTopFocusWindow()
        xStart = xDialog.getChild("startValue")
        xStart.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "1")
        #then "Ctrl+Z"/ undo
        self.xUITest.executeCommand(".uno:Undo")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
