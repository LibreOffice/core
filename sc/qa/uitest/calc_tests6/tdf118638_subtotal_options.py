# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
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
        with self.ui_test.load_file(get_url_for_data_file("tdf118638.ods")) as calc_doc:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")
            # Open the test file
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:B15"}))
            #Data->Subtotals
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "Store Name")
                xCheckListMenu = xDialog.getChild("grid1")
                xTreeList = xCheckListMenu.getChild("columns1")
                xEntry = xTreeList.getChild("1")
                xEntry.executeAction("CLICK", tuple())

                #use the SUM function
        #        xfunctions = xDialog.getChild("functions")
        #        propsF = {"TEXT": "Sum"}
        #        actionPropsF = mkPropertyValues(propsF)
        #        xfunctions.executeAction("SELECT", actionPropsF)


            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 15).getString(), "5408 Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 16).getString(), "Grand Sum")

            self.assertEqual(round(get_cell_by_position(calc_doc, 0, 1, 15).getValue(),12), 238.89)
            self.assertEqual(round(get_cell_by_position(calc_doc, 0, 1, 16).getValue(),12), 238.89)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 15).getString(), "$238.89")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 16).getString(), "$238.89")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
