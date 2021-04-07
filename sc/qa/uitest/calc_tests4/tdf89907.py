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
#Bug 89907 - Text to columns only affects first line when width is auto set

class tdf89907(UITestCase):
    def test_tdf89907_text_to_columns(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #Add data
        enter_text_to_cell(gridwin, "A1", "afasdfs.fdfasd.fsadf.fasd")
        enter_text_to_cell(gridwin, "A2", "3242.43242.3242.2342")
        enter_text_to_cell(gridwin, "A3", "fdsfa.afsdfa.adfdas.fsad")
        enter_text_to_cell(gridwin, "A4", "21312.1111.1111.111")
        #select column A
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")
        #Optimal Width
        self.xUITest.executeCommand(".uno:SetOptimalColumnWidthDirect")
        # Data - Text to Columns
        self.ui_test.execute_dialog_through_command(".uno:TextToColumns")
        xDialog = self.xUITest.getTopFocusWindow()
        xother = xDialog.getChild("other")
        xinputother = xDialog.getChild("inputother")
        if (get_state_as_dict(xother)["Selected"]) == "false":
            xother.executeAction("CLICK", tuple())
        xinputother.executeAction("TYPE", mkPropertyValues({"TEXT":"."}))
        # Click Ok
        xOK = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOK)

        #Verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "afasdfs")
        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 3242)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getString(), "fdsfa")
        self.assertEqual(get_cell_by_position(document, 0, 0, 3).getValue(), 21312)
        self.assertEqual(get_cell_by_position(document, 0, 1, 0).getString(), "fdfasd")
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 43242)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getString(), "afsdfa")
        self.assertEqual(get_cell_by_position(document, 0, 1, 3).getValue(), 1111)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getString(), "fsadf")
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 3242)
        self.assertEqual(get_cell_by_position(document, 0, 2, 2).getString(), "adfdas")
        self.assertEqual(get_cell_by_position(document, 0, 2, 3).getValue(), 1111)
        self.assertEqual(get_cell_by_position(document, 0, 3, 0).getString(), "fasd")
        self.assertEqual(get_cell_by_position(document, 0, 3, 1).getValue(), 2342)
        self.assertEqual(get_cell_by_position(document, 0, 3, 2).getString(), "fsad")
        self.assertEqual(get_cell_by_position(document, 0, 3, 3).getValue(), 111)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
