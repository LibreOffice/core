# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import select_by_text
from uitest.uihelper.common import get_url_for_data_file

#Bug 107267 - During a sub-total calculation or sum, the data set is not computed well.
class Subtotals(UITestCase):


    def test_tdf107267(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf107267.ods"))
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = XcalcDoc.getChild("grid_window")
        # 1. Open the test file
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F123"}))
        # 2. Data->Subtotals
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        # = 1st group = 3. Group by "Person Number", select "shipping time" and use the Max function.
        xGroupBy = xDialog.getChild("group_by")
        select_by_text(xGroupBy, "Person Number")
        # 4. Tick 'Calculate subtotals for' -> "shipping time" - already selected
#        xCheckListMenu = xDialog.getChild("grid1")
#        xTreeList = xCheckListMenu.getChild("columns")
#        x6Entry = xTreeList.getChild("5")
#        xFirstEntry.executeAction("CLICK", tuple())
        #use the Max function

        #= 2nd group =5. Group by "Person Number", select "shipping time" and use the Min function.
        select_pos(xTabs, "1")
        xGroupBy = xDialog.getChild("group_by")
        select_by_text(xGroupBy, "Person Number")
        # 4. Tick 'Calculate subtotals for' -> "shipping time" - already selected
#        xCheckListMenu = xDialog.getChild("grid1")
#        xTreeList = xCheckListMenu.getChild("columns")
#        x6Entry = xTreeList.getChild("5")
#        xFirstEntry.executeAction("CLICK", tuple())
        #use the Min function

        #= 3rd group = Group by "Person Number", select "shipping time" and use the Average function.
        select_pos(xTabs, "2")
        xGroupBy = xDialog.getChild("group_by")
        select_by_text(xGroupBy, "Person Number")
        # 4. Tick 'Calculate subtotals for' -> "shipping time" - already selected
#        xCheckListMenu = xDialog.getChild("grid1")
#        xTreeList = xCheckListMenu.getChild("columns")
#        x6Entry = xTreeList.getChild("5")
#        xFirstEntry.executeAction("CLICK", tuple())
        #use the Average function

        # 5. Click OK
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 141).getString(), "Grand Average")
        self.assertEqual(get_cell_by_position(document, 0, 0, 142).getString(), "Grand Min")
        self.assertEqual(get_cell_by_position(document, 0, 0, 143).getString(), "Grand Max")

        self.assertEqual(round(get_cell_by_position(document, 0, 5, 141).getValue(), 13), 3.3852459016393)
        self.assertEqual(get_cell_by_position(document, 0, 5, 142).getValue(), 0)
        self.assertEqual(get_cell_by_position(document, 0, 5, 143).getValue(), 26)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
