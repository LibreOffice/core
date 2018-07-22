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
#Bug 65686 - EDITING Data > Validity broken (for list entries at least)
class tdf65686(UITestCase):
    def test_tdf65686_validity_list(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        #- Data > Validity
        #- Select Allow List
        #- Enter Entries Aap Noot Mies
        #- OK
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        minlist = xDialog.getChild("minlist")

        props = {"TEXT": "List"}
        actionProps = mkPropertyValues(props)
        xallow.executeAction("SELECT", actionProps)
        minlist.executeAction("TYPE", mkPropertyValues({"TEXT":"Aap"}))
        minlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        minlist.executeAction("TYPE", mkPropertyValues({"TEXT":"Noot"}))
        minlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        minlist.executeAction("TYPE", mkPropertyValues({"TEXT":"Mies"}))
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        #- again open Data > Validity
        #> there are empty lines in the list Entries
        self.ui_test.execute_dialog_through_command(".uno:Validation")
        xDialog = self.xUITest.getTopFocusWindow()
        xallow = xDialog.getChild("allow")
        xallowempty = xDialog.getChild("allowempty")
        minlist = xDialog.getChild("minlist")

        self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "List")
        self.assertEqual(get_state_as_dict(minlist)["Text"], "Aap\nNoot\nMies")

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
