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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf169108(UITestCase):

    def test_tdf169108_prevent_selection_change(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            # Insert test data
            enter_text_to_cell(xGridWindow, "A1", "Header")
            enter_text_to_cell(xGridWindow, "A2", "1")
            enter_text_to_cell(xGridWindow, "A3", "2")
            enter_text_to_cell(xGridWindow, "A4", "1")
            enter_text_to_cell(xGridWindow, "C1", "Header")
            enter_text_to_cell(xGridWindow, "C2", "1")

            # Apply auto filter on C1:C2
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "C1:C2"}))
            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            # Create a pivot table on different range
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
            with self.ui_test.execute_dialog_through_command(".uno:DataDataPilotRun", close_button="cancel"):
                pass

            # Without the fix in place, this test would have failed with
            # AssertionError: 'Sheet1.A1:Sheet1.A4' != 'Sheet1.C1:Sheet1.C2'
            # i.e., the old auto filter range will be restored
            gridWinState = get_state_as_dict(xGridWindow)
            self.assertEqual("Sheet1.A1:Sheet1.A4", gridWinState["MarkedArea"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
