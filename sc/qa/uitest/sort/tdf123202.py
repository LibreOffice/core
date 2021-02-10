# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_row
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 123202 - EDITING: Sorting is incorrect when the data range contents hidden rows

class tdf123202(UITestCase):
    def test_tdf123202_sortdescending_undo(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #In column A enter texts
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "2")
        enter_text_to_cell(gridwin, "A3", "3")
        enter_text_to_cell(gridwin, "A4", "4")
        #select A3
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        #format > Row > Hide
        self.ui_test._xUITest.executeCommand(".uno:HideRow")
        #select A1:A4
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
        #Press toolbarbutton for descending sorting .uno:SortDescending
        self.xUITest.executeCommand(".uno:SortDescending")
        row1 = get_row(document, 1)
        row2 = get_row(document, 2)
        # "3" is not visible
        self.assertFalse(row1.getPropertyValue("IsVisible"))
        # "2" is visible
        self.assertTrue(row2.getPropertyValue("IsVisible"))
        #Undo
        self.xUITest.executeCommand(".uno:Undo")
        # "2" is visible
        self.assertTrue(row1.getPropertyValue("IsVisible"))
        # "3" is not visible
        self.assertFalse(row2.getPropertyValue("IsVisible"))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
