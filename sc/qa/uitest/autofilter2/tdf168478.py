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
from libreoffice.calc.document import is_row_hidden
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf168478(UITestCase):

    def test_tdf168478_restore_multiple_selection(self):
        with self.ui_test.create_doc_in_start_center("calc") as calcDoc:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWindow = xCalcDoc.getChild("grid_window")

            # Insert test data
            enter_text_to_cell(xGridWindow, "A1", "Header")
            enter_text_to_cell(xGridWindow, "A2", "1")
            enter_text_to_cell(xGridWindow, "A3", "2")
            enter_text_to_cell(xGridWindow, "A4", "1")

            # Apply auto filter on A1:A4
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))
            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            # Search for the first entry and apply the auto filter
            xGridWindow.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xTreeList = xFloatWindow.getChild("check_list_box")
            xSearchEdit = xFloatWindow.getChild("search_edit")
            xSearchEdit.executeAction("TYPE", mkPropertyValues({"TEXT" : "1"}))
            self.ui_test.wait_until_property_is_updated(xTreeList, "Children", str(1))
            self.assertEqual("1", get_state_as_dict(xTreeList.getChild('0'))['Text'])
            self.assertEqual(1, len(xTreeList.getChildren()))

            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            # Check that the correct rows are hidden/visible
            self.assertFalse(is_row_hidden(calcDoc, 0))
            self.assertFalse(is_row_hidden(calcDoc, 1))
            self.assertTrue(is_row_hidden(calcDoc, 2))
            self.assertFalse(is_row_hidden(calcDoc, 3))

            # Select more than one cell, remove filter and verify restored range includes all cells
            xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "A2:A4"}))
            self.xUITest.executeCommand(".uno:DataFilterRemoveFilter")
            gridWinState = get_state_as_dict(xGridWindow)

            # Without the fix in place, this test would have failed with
            # AssertionError: 'true' != 'false'
            # i.e., not all cells have been restored
            self.assertFalse(is_row_hidden(calcDoc, 2))

# vim: set shiftwidth=4 softtabstop=4 expandtab:
