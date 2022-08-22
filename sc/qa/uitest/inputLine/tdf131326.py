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

# Bug 131326 - Formula to Value does not automatically change text in input line
class tdf131326(UITestCase):

    def test_tdf131326_formula_to_value_input_line(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "=1")
            self.xUITest.executeCommand(".uno:ConvertFormulaToValue")
            xToolkit = self.xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
            xToolkit.processEventsToIdle()
            xInputWin = xCalcDoc.getChild("sc_input_window")
            self.assertEqual(get_state_as_dict(xInputWin)["Text"], "1")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
