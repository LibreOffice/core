# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 89958 - Data->Filter->Standard Filter, condition "does not end with" does filter too much

class tdf89958(UITestCase):
    def test_td89958_standard_filter(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf89958.ods"))
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        #select A1-> Column .uno:SelectColumn
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        self.xUITest.executeCommand(".uno:SelectColumn")

        #Menu: Data->Filter->Standard Filter ...
        #Field Name "Column A", Condition "Does not end with", Value: "CTORS"
        self.ui_test.execute_modeless_dialog_through_command(".uno:DataFilterStandardFilter")
        xDialog = self.xUITest.getTopFocusWindow()
        xfield1 = xDialog.getChild("field1")
        xval1 = xDialog.getChild("val1")
        xcond1 = xDialog.getChild("cond1")

        props = {"TEXT": "Column A"}
        actionProps = mkPropertyValues(props)
        xfield1.executeAction("SELECT", actionProps)
        props2 = {"TEXT": "Does not end with"}
        actionProps2 = mkPropertyValues(props2)
        xcond1.executeAction("SELECT", actionProps2)
        xval1.executeAction("TYPE", mkPropertyValues({"TEXT":"CTORS"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

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

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab: