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
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues

#Bug 123122 - Invoking the number format dialog the format code field is blank for a user-defined date+time format.

class tdf123122(UITestCase):
    def test_tdf123122_format_cell_datetime(self):
        #numberingformatpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            #select cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers
                xformatted = xDialog.getChild("formatted")

                #Format cells and enter NN MMM DD YYYY HH:MM as format code
                xformatted.executeAction("CLEAR", tuple())  #clear textbox
                xformatted.executeAction("TYPE", mkPropertyValues({"TEXT":"NN MMM DD YYYY HH:MM"}))
                #save - OK

            #verify
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers
                xliststore1 = xDialog.getChild("categorylb")  #1st list / Category
                xformatted = xDialog.getChild("formatted")


                self.assertEqual(get_state_as_dict(xliststore1)["SelectEntryText"], "Date")
                self.assertEqual(get_state_as_dict(xformatted)["Text"], "NN MMM DD YYYY HH:MM")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
