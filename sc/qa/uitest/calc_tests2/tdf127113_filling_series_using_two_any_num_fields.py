# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf127113(UITestCase):

    def launch_dialog(self, direction):
        with self.ui_test.execute_dialog_through_command(".uno:FillSeries") as xDialog:
            xDirection = xDialog.getChild(direction)
            xDirection.executeAction("CLICK", tuple())

            xStart = xDialog.getChild("startValue")
            xStart.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
            self.assertEqual("1", get_state_as_dict(xStart)["Text"])

            xEnd = xDialog.getChild("endValue")
            xEnd.executeAction("TYPE", mkPropertyValues({"TEXT":"5"}))
            self.assertEqual("5", get_state_as_dict(xEnd)["Text"])

            # Without the fix in place, this test would have failed because the increment child
            # couldn't be empty and a warning dialog would be displayed
            xIncrement = xDialog.getChild("increment")
            xIncrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
            xIncrement.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
            self.assertEqual("", get_state_as_dict(xIncrement)["Text"])

    def test_tdf127113(self):
        with self.ui_test.create_doc_in_start_center("calc") as calc_doc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A9"}))

            self.launch_dialog("down")

            value = 1.0
            for i in range(9):
                self.assertEqual(value, get_cell_by_position(calc_doc, 0, 0, i).getValue())
                value += 0.5

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B9"}))

            self.launch_dialog("up")

            for i in range(9):
                value -= 0.5
                self.assertEqual(value, get_cell_by_position(calc_doc, 0, 1, i).getValue())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
