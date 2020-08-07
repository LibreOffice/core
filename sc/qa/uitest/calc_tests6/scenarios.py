# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.calc import enter_text_to_cell

# uitest Tools - Scenarios

class scenarios(UITestCase):

   def test_scenarios(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        xGridWindow = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #fill B1:B4  with 1, select B1:B4;
        enter_text_to_cell(xGridWindow, "B1", "1")
        enter_text_to_cell(xGridWindow, "B2", "1")
        enter_text_to_cell(xGridWindow, "B3", "1")
        enter_text_to_cell(xGridWindow, "B4", "1")
        xGridWindow.executeAction("SELECT", mkPropertyValues({"RANGE": "B1:B4"}))
        #scenarios - scenario A1
        self.ui_test.execute_dialog_through_command(".uno:ScenarioManager")
        xDialog = self.xUITest.getTopFocusWindow()

        name = xDialog.getChild("name")
        name.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        name.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        name.executeAction("TYPE", mkPropertyValues({"TEXT":"A1"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #next scenarios - scenario B1
        self.ui_test.execute_dialog_through_command(".uno:ScenarioManager")
        xDialog = self.xUITest.getTopFocusWindow()

        name = xDialog.getChild("name")
        name.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        name.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        name.executeAction("TYPE", mkPropertyValues({"TEXT":"B1"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()
# vim: set shiftwidth=4 softtabstop=4 expandtab:
