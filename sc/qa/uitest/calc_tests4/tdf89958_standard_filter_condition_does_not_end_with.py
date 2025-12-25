# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_by_text
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 89958 - Data->Filter->Standard Filter, condition "does not end with" does filter too much

class tdf89958(UITestCase):
    def test_td89958_standard_filter(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf89958.ods")):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select A1-> Column .uno:SelectColumn
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            self.xUITest.executeCommand(".uno:SelectColumn")

            #Menu: Data->Filter->Standard Filter ...
            #Field Name "Column A", Condition "Does not end with", Value: "CTORS"
            with self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter") as xDialog:
                xfield1 = xDialog.getChild("field1")
                xval1 = xDialog.getChild("val1")
                xcond1 = xDialog.getChild("cond1")

                select_by_text(xfield1, "Column A")
                select_by_text(xcond1, "Does not end with")
                xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"CTORS"}))

            #Expected behaviours: A2 is not filtered as it does not end with "CTORS".
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "1")
            gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "DOWN"}))
            gridWinState = get_state_as_dict(gridwin)
            self.assertEqual(gridWinState["CurrentRow"], "3")
    #        #reopen filter and verify - doesn't works
    #        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
    #        gridwin.executeAction("TYPE", mkPropertyValues({"KEYCODE": "UP"}))
    #        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter")
    #        xDialog = self.xUITest.getTopFocusWindow()
    #        xfield1 = xDialog.getChild("field1")
    #        xval1 = xDialog.getChild("val1")
    #        xcond1 = xDialog.getChild("cond1")
    #        self.assertEqual(get_state_as_dict(xfield1)["SelectEntryText"], "Column A")
    #        self.assertEqual(get_state_as_dict(xval1)["Text"], "CTORS")
    #        self.assertEqual(get_state_as_dict(xcond1)["SelectEntryText"], "Does not end with")
    #        xCancelBtn = xDialog.getChild("cancel")
    #        self.ui_test.close_dialog_through_button(xCancelBtn)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
