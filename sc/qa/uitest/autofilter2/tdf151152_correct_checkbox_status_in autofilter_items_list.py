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

#Bug 151152 - Autofilter checkbox status is wrong when I deselect one item, click OK, then click on the dropdown again (2nd case)

class tdf151152(UITestCase):
    def test_tdf151152(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "A2", "0")
            enter_text_to_cell(gridwin, "A3", "a")
            enter_text_to_cell(gridwin, "A4", "b")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            xEntry = xList.getChild("1")
            xEntry.executeAction("CLICK", tuple())

            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(3, len(xList.getChildren()))
            self.assertEqual('true', get_state_as_dict(xList.getChild('0'))['IsChecked'])
            self.assertEqual('false', get_state_as_dict(xList.getChild('1'))['IsChecked'])
            self.assertEqual('true', get_state_as_dict(xList.getChild('2'))['IsChecked'])
            xCloseBtn = xFloatWindow.getChild("cancel")
            xCloseBtn.executeAction("CLICK", tuple())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
