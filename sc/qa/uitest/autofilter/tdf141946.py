# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 141946 - EDITING Inserting column before autofiltered column empties autofilter

class tdf141946(UITestCase):
    def test_tdf141946_inserted_column(self):
        self.ui_test.create_doc_in_start_center("calc")
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        gridwin = calcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "A")
        enter_text_to_cell(gridwin, "A2", "1")
        enter_text_to_cell(gridwin, "A3", "2")
        enter_text_to_cell(gridwin, "A4", "3")

        enter_text_to_cell(gridwin, "B1", "B")
        enter_text_to_cell(gridwin, "B2", "4")
        enter_text_to_cell(gridwin, "B3", "5")
        enter_text_to_cell(gridwin, "B4", "6")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B4"}))

        self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        xEntry = xList.getChild("1")
        xEntry.executeAction("CLICK", tuple())

        xOkButton = xFloatWindow.getChild("ok")
        xOkButton.executeAction("CLICK", tuple())

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))

        self.xUITest.executeCommand(".uno:InsertColumnsBefore")

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(2, len(xList.getChildren()))
        xCloseBtn = xFloatWindow.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(1, len(xList.getChildren()))
        xCloseBtn = xFloatWindow.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "2", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        self.assertEqual(3, len(xList.getChildren()))
        xCloseBtn = xFloatWindow.getChild("cancel")
        xCloseBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
