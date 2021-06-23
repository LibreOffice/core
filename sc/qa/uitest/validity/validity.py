# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

class validity(UITestCase):
    def test_validity_tab_criteria(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xdata = xDialog.getChild("data")
        xmin = xDialog.getChild("min")
        xmax = xDialog.getChild("max")

        select_by_text(xallow, "Whole Numbers")
        xallowempty.executeAction("CLICK", tuple())
        select_by_text(xdata, "valid range")
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
        xmax.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #reopen and verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xdata = xDialog.getChild("data")
        xmin = xDialog.getChild("min")
        xmax = xDialog.getChild("max")

        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Whole Numbers")
        self.assertEqual(get_state_as_dict(xallowempty)["Selected"], "false")
        self.assertEqual(get_state_as_dict(xdata)["SelectEntryText"], "valid range")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "1")
        self.assertEqual(get_state_as_dict(xmax)["Text"], "2")
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_validity_tab_inputHelp(self):
        #validationhelptabpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "1")
        xtsbhelp = xDialog.getChild("tsbhelp")
        xtitle = xDialog.getChild("title")
        xinputhelp = xDialog.getChild("inputhelp")

        xtsbhelp.executeAction("CLICK", tuple())
        xtitle.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))
        xinputhelp.executeAction("TYPE", mkPropertyValues({"TEXT":"B"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #reopen and verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        xtsbhelp = xDialog.getChild("tsbhelp")
        xtitle = xDialog.getChild("title")
        xinputhelp = xDialog.getChild("inputhelp")
        # print(get_state_as_dict(xtsbhelp))
        select_pos(xTabs, "1")
        self.assertEqual(get_state_as_dict(xtsbhelp)["Selected"], "true")
        self.assertEqual(get_state_as_dict(xtitle)["Text"], "A")
        self.assertEqual(get_state_as_dict(xinputhelp)["Text"], "B")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

    def test_validity_tab_errorAlert(self):
        # erroralerttabpage.ui
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")

        xactionCB = xDialog.getChild("actionCB")
        xerroralerttitle = xDialog.getChild("erroralert_title")
        xerrorMsg = xDialog.getChild("errorMsg")

        select_by_text(xactionCB, "Warning")
        xerroralerttitle.executeAction("TYPE", mkPropertyValues({"TEXT":"Warn"}))
        xerrorMsg.executeAction("TYPE", mkPropertyValues({"TEXT":"Warn2"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #reopen and verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "2")

        xactionCB = xDialog.getChild("actionCB")
        xerroralerttitle = xDialog.getChild("erroralert_title")
        xerrorMsg = xDialog.getChild("errorMsg")

        self.assertEqual(get_state_as_dict(xactionCB)["SelectEntryText"], "Warning")
        self.assertEqual(get_state_as_dict(xerroralerttitle)["Text"], "Warn")
        self.assertEqual(get_state_as_dict(xerrorMsg)["Text"], "Warn2")
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)


        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
