# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import select_by_text, select_pos
from uitest.uihelper.common import get_url_for_data_file
from uitest.uihelper.common import get_state_as_dict

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class Subtotals(UITestCase):

    def test_tdf162262(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf162262.ods")) as calc_doc:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")
            # One group level
            # Select cell range
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C15"}))
            # Select from the menu bar Data
            # Select option subtotal
            # Subtotal dialog displays
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                # Select group by: Day
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "Day")
                # Select 'Calculate subtotals for' -> Value 1 and Value 2
                xCheckListMenu = xDialog.getChild("grid1")
                xTreeList = xCheckListMenu.getChild("columns1")
                xFirstEntry = xTreeList.getChild("1")
                xFirstEntry.executeAction("CLICK", tuple())
                xFirstEntry = xTreeList.getChild("2")
                xFirstEntry.executeAction("CLICK", tuple())

                # Select tab options
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")
                # Unselect option Summary below -> false
                xSummarybelow = xDialog.getChild("summarybelow")
                xSummarybelow.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xSummarybelow)['Selected'], "false")
                # apply with OK

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "Grand Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 105)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 119)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getString(), "Friday Result")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 19)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 21)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 20).getString(), "Wednesday Result")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 20).getValue(), 11)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 20).getValue(), 13)

    def test_tdf162262_multi(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf162262.ods")) as calc_doc:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")
            # Multi group level
            # Select cell range
            gridwin.executeAction("SELECT", mkPropertyValues({"RANGE": "A1:C15"}))
            # Select from the menu bar Data
            # Select option subtotal
            # Subtotal dialog displays
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                # Select group by 1: Day
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "Day")
                # Select 'Calculate subtotals for' -> Value 1
                xCheckListMenu = xDialog.getChild("grid1")
                xTreeList = xCheckListMenu.getChild("columns1")
                # Select 1 column
                xFirstEntry = xTreeList.getChild("1")
                xFirstEntry.executeAction("CLICK", tuple())

                # Select tab Group by 2
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")

                # Select group by 2: Day
                xGroupBy = xDialog.getChild("group_by2")
                select_by_text(xGroupBy, "Day")
                # Select 'Calculate subtotals for' -> Value 2
                xCheckListMenu = xDialog.getChild("grid2")
                xTreeList = xCheckListMenu.getChild("columns2")
                # Select second column
                xFirstEntry = xTreeList.getChild("2")
                xFirstEntry.executeAction("CLICK", tuple())

                # Select tab options
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")
                # Unselect option Summary below -> false
                xSummarybelow = xDialog.getChild("summarybelow")
                xSummarybelow.executeAction("CLICK", tuple())
                self.assertEqual(get_state_as_dict(xSummarybelow)['Selected'], "false")
                # apply with OK

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "Grand Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), 119)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getString(), "Grand Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 2).getValue(), 105)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 2).getValue(), 0)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getString(), "Friday Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 3).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 3).getValue(), 21)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getString(), "Friday Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 4).getValue(), 19)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 4).getValue(), 0)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 27).getString(), "Wednesday Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 27).getValue(), 0)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 27).getValue(), 13)

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 28).getString(), "Wednesday Sum")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 28).getValue(), 11)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 28).getValue(), 0)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
