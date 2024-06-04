# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_by_text, select_pos

from libreoffice.uno.propertyvalue import mkPropertyValues


class validity(UITestCase):
    def test_validity_tab_criteria(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xdata = xDialog.getChild("data")
                xmin = xDialog.getChild("min")
                xmax = xDialog.getChild("max")

                select_by_text(xallow, "Integer")
                xallowempty.executeAction("CLICK", tuple())
                select_by_text(xdata, "valid range")
                xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"1"}))
                xmax.executeAction("TYPE", mkPropertyValues({"TEXT":"2"}))
            #reopen and verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation", close_button="cancel") as xDialog:
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xdata = xDialog.getChild("data")
                xmin = xDialog.getChild("min")
                xmax = xDialog.getChild("max")

                self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Integer")
                self.assertEqual(get_state_as_dict(xallowempty)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xdata)["SelectEntryText"], "valid range")
                self.assertEqual(get_state_as_dict(xmin)["Text"], "1")
                self.assertEqual(get_state_as_dict(xmax)["Text"], "2")


    def test_validity_tab_inputHelp(self):
        #validationhelptabpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")
                xtsbhelp = xDialog.getChild("tsbhelp")
                xtitle = xDialog.getChild("title")
                xinputhelp = xDialog.getChild("inputhelp_text")

                xtsbhelp.executeAction("CLICK", tuple())
                xtitle.executeAction("TYPE", mkPropertyValues({"TEXT":"A"}))
                xinputhelp.executeAction("TYPE", mkPropertyValues({"TEXT":"B"}))
            #reopen and verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                xtsbhelp = xDialog.getChild("tsbhelp")
                xtitle = xDialog.getChild("title")
                xinputhelp = xDialog.getChild("inputhelp_text")
                select_pos(xTabs, "1")
                self.assertEqual(get_state_as_dict(xtsbhelp)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xtitle)["Text"], "A")
                self.assertEqual(get_state_as_dict(xinputhelp)["Text"], "B")


    def test_validity_tab_errorAlert(self):
        # erroralerttabpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))

            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                xactionCB = xDialog.getChild("actionCB")
                xerroralerttitle = xDialog.getChild("erroralert_title")
                xerrorMsg = xDialog.getChild("errorMsg")

                select_by_text(xactionCB, "Warning")
                xerroralerttitle.executeAction("TYPE", mkPropertyValues({"TEXT":"Warn"}))
                xerrorMsg.executeAction("TYPE", mkPropertyValues({"TEXT":"Warn2"}))
            #reopen and verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "2")

                xactionCB = xDialog.getChild("actionCB")
                xerroralerttitle = xDialog.getChild("erroralert_title")
                xerrorMsg = xDialog.getChild("errorMsg")

                self.assertEqual(get_state_as_dict(xactionCB)["SelectEntryText"], "Warning")
                self.assertEqual(get_state_as_dict(xerroralerttitle)["Text"], "Warn")
                self.assertEqual(get_state_as_dict(xerrorMsg)["Text"], "Warn2")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
