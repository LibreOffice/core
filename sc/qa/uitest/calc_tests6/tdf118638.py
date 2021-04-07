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
#Bug 118638 - subtotal option loses doesn't contain existing format for column
class Subtotals(UITestCase):


    def test_tdf118638_subtotal_format(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf118638.ods"))
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = XcalcDoc.getChild("grid_window")
        # Open the test file
        gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B15"}))
        #Data->Subtotals
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")
        xGroupBy = xDialog.getChild("group_by")
        select_by_text(xGroupBy, "Store Name")
        xCheckListMenu = xDialog.getChild("grid1")
        xTreeList = xCheckListMenu.getChild("columns")
        xEntry = xTreeList.getChild("1")
        xEntry.executeAction("CLICK", tuple())

        #use the SUM function
#        xfunctions = xDialog.getChild("functions")
#        propsF = {"TEXT": "Sum"}
#        actionPropsF = mkPropertyValues(propsF)
#        xfunctions.executeAction("SELECT", actionPropsF)

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        #verify
        self.assertEqual(get_cell_by_position(document, 0, 0, 15).getString(), "5408 Sum")
        self.assertEqual(get_cell_by_position(document, 0, 0, 16).getString(), "Grand Sum")

        self.assertEqual(get_cell_by_position(document, 0, 1, 15).getValue(), 238.89)
        self.assertEqual(get_cell_by_position(document, 0, 1, 16).getValue(), 238.89)

        self.assertEqual(get_cell_by_position(document, 0, 1, 15).getString(), "$238.89")
        self.assertEqual(get_cell_by_position(document, 0, 1, 16).getString(), "$238.89")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
