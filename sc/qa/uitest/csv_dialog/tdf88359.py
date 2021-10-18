# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos
from libreoffice.calc.csv_dialog import load_csv_file
from libreoffice.uno.propertyvalue import mkPropertyValues

class Tdf88359(UITestCase):

    def test_tdf88359(self):

        # Import the file with 'Detect Special Numbers' unchecked
        with load_csv_file(self, "tdf88359.csv", True):
            pass

        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))

        with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
            xTabs = xDialog.getChild("tabcontrol")
            select_pos(xTabs, "0")
            xliststore1 = xDialog.getChild("categorylb")
            xliststore2 = xDialog.getChild("formatlb")
            xformatted = xDialog.getChild("formatted")

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Date' != 'Text'
            self.assertEqual("Date", get_state_as_dict(xliststore1)["SelectEntryText"])
            self.assertEqual("1999-12-31T13:37:46", get_state_as_dict(xliststore2)["SelectEntryText"])
            self.assertEqual("YYYY-MM-DD\"T\"HH:MM:SS", get_state_as_dict(xformatted)["Text"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
