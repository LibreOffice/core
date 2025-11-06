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
from uitest.uihelper.common import get_url_for_data_file, get_state_as_dict

#Bug 107267 - During a sub-total calculation or sum, the data set is not computed well.
class Subtotals(UITestCase):


    def test_tdf107267(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf107267.ods")) as calc_doc:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")
            # 1. Open the test file
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:F123"}))
            # 2. Data->Subtotals
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")

                select_pos(xTabs, "0")
                xGroupBy1 = xDialog.getChild("group_by1")
                self.assertEqual("Person Number", get_state_as_dict(xGroupBy1)["SelectEntryText"])

                xColumns1 = xDialog.getChild("columns1")
                self.assertEqual("shipping time", get_state_as_dict(xColumns1)["SelectEntryText"])

                select_pos(xTabs, "1")
                xGroupBy2 = xDialog.getChild("group_by2")
                self.assertEqual("Person Number", get_state_as_dict(xGroupBy2)["SelectEntryText"])

                xColumns2 = xDialog.getChild("columns2")
                self.assertEqual("shipping time", get_state_as_dict(xColumns2)["SelectEntryText"])

                select_pos(xTabs, "2")
                xGroupBy3 = xDialog.getChild("group_by3")
                self.assertEqual("Person Number", get_state_as_dict(xGroupBy3)["SelectEntryText"])

                xColumns3 = xDialog.getChild("columns3")
                self.assertEqual("shipping time", get_state_as_dict(xColumns3)["SelectEntryText"])

            #verify
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 141).getString(), "Grand Average")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 142).getString(), "Grand Min")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 143).getString(), "Grand Max")

            self.assertEqual(round(get_cell_by_position(calc_doc, 0, 5, 141).getValue(), 13), 3.3852459016393)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 142).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 5, 143).getValue(), 26)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
