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


# Bug 96698 - Data => Validity => Custom (like Excel) is missing
class tdf96698(UITestCase):
    def test_tdf96698_validity_custom_formula(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")

            #A general validity check for the entered new content of the active cell - especially for text
            #with a custom formula like in Excel is not possible.
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A2"}))
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                select_by_text(xallow, "Custom")
                xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"ISERROR(FIND(\",\",B2))"}))
            #verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
                self.assertEqual(get_state_as_dict(xmin)["Text"], "ISERROR(FIND(\",\",B2))")


            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A3"}))
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                select_by_text(xallow, "Custom")
                xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"NOT(ISERROR(B3))"}))
            #verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
                self.assertEqual(get_state_as_dict(xmin)["Text"], "NOT(ISERROR(B3))")


            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A7"}))
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                select_by_text(xallow, "Custom")
                xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"ISERROR(FIND(\",\",A7))"}))
            #verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
                self.assertEqual(get_state_as_dict(xmin)["Text"], "ISERROR(FIND(\",\",A7))")


            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A8"}))
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                select_by_text(xallow, "Custom")
                xmin.executeAction("TYPE", mkPropertyValues({"TEXT":"NOT(ISERROR(A8))"}))
            #verify
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xallow = xDialog.getChild("allow")
                xallowempty = xDialog.getChild("allowempty")
                xmin = xDialog.getChild("min")

                self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "Custom")
                self.assertEqual(get_state_as_dict(xmin)["Text"], "NOT(ISERROR(A8))")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
