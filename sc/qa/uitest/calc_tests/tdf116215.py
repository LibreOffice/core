# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
import os
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_sheet_from_doc
from libreoffice.calc.conditional_format import get_conditional_format_from_sheet
from uitest.debug import sleep
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug: Sum button: Calc adds wrong sum formulas if you select more than one row or column

class tdf116215(UITestCase):

    def test_tdf116215_autosum_wrong_more_rows_columns(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        enter_text_to_cell(gridwin, "A1", "1")
        enter_text_to_cell(gridwin, "A2", "1")
        enter_text_to_cell(gridwin, "B1", "1")
        enter_text_to_cell(gridwin, "B2", "1")
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C3"}))
        self.xUITest.executeCommand(".uno:AutoSum")

        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 0, 2).getFormula(), "=SUM(A1:A2)")
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 1, 2).getFormula(), "=SUM(B1:B2)")
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 0).getFormula(), "=SUM(A1:B1)")
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), 2)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getFormula(), "=SUM(A2:B2)")
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
