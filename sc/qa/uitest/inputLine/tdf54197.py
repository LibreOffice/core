# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 54197 - Calc single cell filling (Ctrl+D) does not change data at input line
class tdf54197(UITestCase):

    def test_tdf54197_CTRL_D_input_line_change(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            # 1. go to cell A1 enter any text
            enter_text_to_cell(gridwin, "A1", "t")
            # 2. go to cell A2 press Ctrl+D
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            self.xUITest.executeCommand(".uno:FillDown")
            # The same text as above is displayed at cell A2, BUT input line is still blank
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "t")
            self.assertEqual(get_cell_by_position(document, 0, 0, 1).getString(), "t")
            xInputWin = xCalcDoc.getChild("sc_input_window")
            self.assertEqual(get_state_as_dict(xInputWin)["Text"], "t")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
