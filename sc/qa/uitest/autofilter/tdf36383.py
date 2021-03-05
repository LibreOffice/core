# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_row

#Bug 36383 - EDITING auto row height or change row height removes AutoFilter result

class tdf36383(UITestCase):
    def test_tdf36383_row_height(self):
        self.ui_test.create_doc_in_start_center("calc")
        document = self.ui_test.get_component()
        calcDoc = self.xUITest.getTopFocusWindow()
        gridwin = calcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        enter_text_to_cell(gridwin, "A1", "A")
        enter_text_to_cell(gridwin, "A2", "1")
        enter_text_to_cell(gridwin, "A3", "2")
        enter_text_to_cell(gridwin, "A4", "3")

        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:A4"}))

        self.xUITest.executeCommand(".uno:DataFilterAutoFilter")

        gridwin.executeAction("LAUNCH", mkPropertyValues({"AUTOFILTER": "", "COL": "0", "ROW": "0"}))
        xFloatWindow = self.xUITest.getFloatWindow()
        xCheckListMenu = xFloatWindow.getChild("check_list_menu")
        xList = xCheckListMenu.getChild("check_list_box")
        xEntry = xList.getChild("1")
        xEntry.executeAction("CLICK", tuple())

        xOkButton = xFloatWindow.getChild("ok")
        xOkButton.executeAction("CLICK", tuple())

        row = get_row(document, 2)
        self.assertFalse(row.getPropertyValue("IsVisible"))

        #row height
        self.ui_test.execute_dialog_through_command(".uno:RowHeight")
        xDialog = self.xUITest.getTopFocusWindow()
        xvalue = xDialog.getChild("value")
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        xvalue.executeAction("TYPE", mkPropertyValues({"TEXT":"1 cm"}))

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.assertFalse(row.getPropertyValue("IsVisible"))

        #optimal row height
        self.ui_test.execute_dialog_through_command(".uno:SetOptimalRowHeight")
        xDialog = self.xUITest.getTopFocusWindow()

        xOk = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOk)

        self.assertFalse(row.getPropertyValue("IsVisible"))

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
