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


# Bug 65686 - EDITING Data > Validity broken (for list entries at least)
class tdf65686(UITestCase):
    def test_tdf65686_validity_list(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #- Data > Validity
            #- Select Allow List
            #- Enter Entries Aap Noot Mies
            #- OK
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xallow = xDialog.getChild("allow")
                minlist = xDialog.getChild("minlist")

                select_by_text(xallow, "List")
                minlist.executeAction("TYPE", mkPropertyValues({"TEXT":"Aap"}))
                minlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                minlist.executeAction("TYPE", mkPropertyValues({"TEXT":"Noot"}))
                minlist.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
                minlist.executeAction("TYPE", mkPropertyValues({"TEXT":"Mies"}))
            #- again open Data > Validity
            #> there are empty lines in the list Entries
            with self.ui_test.execute_dialog_through_command(".uno:Validation") as xDialog:
                xallow = xDialog.getChild("allow")
                minlist = xDialog.getChild("minlist")

                self.assertEqual(get_state_as_dict(xallow)["SelectEntryText"], "List")
                self.assertEqual(get_state_as_dict(minlist)["Text"], "Aap\nNoot\nMies")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
