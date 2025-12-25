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
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 48025 - EDITING AUTOFILTER: Autofilter settings stay with the column number when deleting columns

class tdf48025(UITestCase):
    def test_tdf48025_deleted_columns(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            calcDoc = self.xUITest.getTopFocusWindow()
            gridwin = calcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "A")
            enter_text_to_cell(gridwin, "A2", "1")
            enter_text_to_cell(gridwin, "A3", "2")
            enter_text_to_cell(gridwin, "A4", "3")

            enter_text_to_cell(gridwin, "B1", "B")
            enter_text_to_cell(gridwin, "B2", "4")
            enter_text_to_cell(gridwin, "B3", "5")
            enter_text_to_cell(gridwin, "B4", "6")

            enter_text_to_cell(gridwin, "C1", "C")
            enter_text_to_cell(gridwin, "C2", "7")
            enter_text_to_cell(gridwin, "C3", "8")
            enter_text_to_cell(gridwin, "C4", "9")

            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C4"}))

            self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            xEntry = xList.getChild("1")
            xEntry.executeAction("CLICK", tuple())

            xOkButton = xFloatWindow.getChild("ok")
            xOkButton.executeAction("CLICK", tuple())

            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            self.xUITest.executeCommand(".uno:DeleteColumns")

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            self.assertEqual(3, len(xList.getChildren()))
            xCloseBtn = xFloatWindow.getChild("cancel")
            xCloseBtn.executeAction("CLICK", tuple())

            gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "1", "ROW": "0"}))
            xFloatWindow = self.xUITest.getFloatWindow()
            xCheckListMenu = xFloatWindow.getChild("FilterDropDown")
            xList = xCheckListMenu.getChild("check_list_box")
            # since tdf#117267, we are showing the hidden filter rows as inactive elements (2 active + 1 inactive)
            self.assertEqual(3, len(xList.getChildren()))
            xCloseBtn = xFloatWindow.getChild("cancel")
            xCloseBtn.executeAction("CLICK", tuple())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
