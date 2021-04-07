# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.calc import enter_text_to_cell

#Bug 125030 - nice crash by repeating apply style (Ctrl+Shft+Y)

class tdf125030(UITestCase):
    def test_tdf125030_repeat_crash(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #enter data
        enter_text_to_cell(gridwin, "A1", "aaaa")
        enter_text_to_cell(gridwin, "B1", "bbbb")
        #select A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:StyleApply?Style:string=Heading%201&FamilyName:string=CellStyles")
        #focus on A2
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        #repeat (Ctrl+Shft+Y)
        self.xUITest.executeCommand(".uno:Repeat")
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "aaaa")

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
