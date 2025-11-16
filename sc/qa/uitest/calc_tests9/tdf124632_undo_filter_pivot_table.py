# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from libreoffice.calc.document import get_cell_by_position

class tdf124632(UITestCase):
    def test_undo(self):

        with self.ui_test.load_file(get_url_for_data_file("tdf124632.ods")) as calc_doc:

            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            # 1) Cause all elements to be filtered out of the pivot table:

            gridwin.executeAction("LAUNCH", mkPropertyValues({"PIVOTTABLE": "", "COL": "6", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            xFirstEntry = xTreeList.getChild("1")
            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            gridwin.executeAction("LAUNCH", mkPropertyValues({"PIVOTTABLE": "", "COL": "5", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")
            xFirstEntry = xTreeList.getChild("0")
            xFirstEntry.executeAction("CLICK", tuple())

            xOkBtn = xFloatWindow.getChild("ok")
            xOkBtn.executeAction("CLICK", tuple())

            # 2) Confirm the pivot table is empty:

            self.assertEqual("Total Result", get_cell_by_position(calc_doc, 0, 5, 1).getString())

            # 3) Undo

            self.xUITest.executeCommand(".uno:Undo")

            # 4) Reopen the pivot table filter and observe that it's still in its pre-undo state

            # Before, this would have failed as we wouldn't have restored the DataPilot correctly
            gridwin.executeAction("LAUNCH", mkPropertyValues({"PIVOTTABLE": "", "COL": "5", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()

            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xTreeList = xCheckListMenu.getChild("check_list_box")

            # Before, this would have failed as we wouldn't have restored the DataPilot correctly
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('0'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xTreeList.getChild('1'))['IsChecked'])

            xCancelBtn = xFloatWindow.getChild("cancel")
            xCancelBtn.executeAction("CLICK", tuple())

            # Before, this would have failed as we wouldn't have restored the DataPilot correctly
            gridwin.executeAction("LAUNCH", mkPropertyValues({"PIVOTTABLE": "", "COL": "6", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            self.assertIsNotNone(xFloatWindow)



# vim: set shiftwidth=4 softtabstop=4 expandtab:
