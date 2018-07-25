# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 96698 - Data => Validity => Custom (like Excel) is missing
class tdf96698(UITestCase):
    def test_tdf96698_validity_custom_formula(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

        #A general validity check for the entered new content of the active cell - especially for text
        #with a custom formula like in Excel is not possible.
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        props = {"TEXT": "Custom"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"ISERROR(FIND(\",\",B2))"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "ISERROR(FIND(\",\",B2))")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        props = {"TEXT": "Custom"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"NOT(ISERROR(B3))"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "NOT(ISERROR(B3))")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A7"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        props = {"TEXT": "Custom"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"ISERROR(FIND(\",\",A7))"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "ISERROR(FIND(\",\",A7))")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A8"}))
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        props = {"TEXT": "Custom"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"NOT(ISERROR(A8))"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        xmin = xDialog.getChild("min")

        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
        self.assertEqual(get_state_as_dict(xmin)["Text"], "NOT(ISERROR(A8))")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
