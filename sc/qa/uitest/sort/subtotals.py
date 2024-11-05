# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import select_by_text, select_pos
from uitest.uihelper.common import get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


class Subtotals(UITestCase):

    def test_tdf114720(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")

            enter_text_to_cell(gridwin, "A1", "1")
            enter_text_to_cell(gridwin, "A2", "1")
            enter_text_to_cell(gridwin, "A3", "1")
            enter_text_to_cell(gridwin, "A4", "1")
            enter_text_to_cell(gridwin, "A5", "1")
            enter_text_to_cell(gridwin, "A6", "1")
            enter_text_to_cell(gridwin, "A7", "1")
            enter_text_to_cell(gridwin, "A8", "1")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A9"}))

            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals"):
                pass

            self.assertEqual(get_cell_by_position(document, 0, 0, 7).getValue(), 1)
            self.assertEqual(get_cell_by_position(document, 0, 0, 8).getString(), "")

            # check cancel button
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals", close_button="cancel"):
                pass


    def test_tdf88792(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf88792.ods")) as calc_doc:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")

            # go to cell A1
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            # Select from the menu bar Data
            # Select option subtotal
            # Subtotal dialog displays
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                # Select group by: Category
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "Category")
                # Select calculate subtotals for the months -  selected by default
                # Select tab options
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "3")
                # select option include formats
                xformats = xDialog.getChild("formats")
                xformats.executeAction("CLICK", tuple())
                # apply with OK

            self.assertEqual(get_cell_by_position(calc_doc, 0, 3, 5).getValue(), 28000)

    def test_tdf88735(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf88735.ods")) as calc_doc:
            # 1 select all cells
            self.xUITest.executeCommand(".uno:SelectAll")#use uno command Menu Edit->Select All
            # 2 invoke sub-total menu and select none
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "- none -")
            # 2 invoke sort menu and... crash
            with self.ui_test.execute_dialog_through_command(".uno:DataSort", close_button="cancel"):
                pass

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 8).getString(), "z")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 8).getValue(), 8)

    def test_tdf56958(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf56958.ods")) as calc_doc:
            # 1. Open the test file
            # 2. Data->Subtotals
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                # 3. Group by->Trans date
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "Trans Date")
                # 4. Tick 'Calculate subtotals for' -> Amount  (grid1)
                xCheckListMenu = xDialog.getChild("grid1")
                xTreeList = xCheckListMenu.getChild("columns1")
                xFirstEntry = xTreeList.getChild("2")
                xFirstEntry.executeAction("CLICK", tuple())
                # 5. Click OK
            # 6. Data->Subtotals
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                # 7. Group by->-none-
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "- none -")
                # 8. Untick 'Calculate subtotals for' -> Amount
                xCheckListMenu = xDialog.getChild("grid1")
                xTreeList = xCheckListMenu.getChild("columns1")
                xFirstEntry = xTreeList.getChild("2")
                xFirstEntry.executeAction("CLICK", tuple())
                # 9. Click OK
            # 10. Data->Sort
            with self.ui_test.execute_dialog_through_command(".uno:DataSort") as xDialog:
                # 11. Sort key 1->Post Date.
                sortkey1 = xDialog.getChild("sortlb")
                select_by_text(sortkey1, "Post Date")
                # 12. Sort key 2->-undefined-
                sortkey2 = xDialog.getChild("sortuserlb")
                select_by_text(sortkey2, "- undefined -")
                # 13. Click OK
            self.assertEqual(get_cell_by_position(calc_doc, 0, 2, 1).getValue(), -0.25)

    def test_tdf55734(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf55734.ods")) as calc_doc:
            XcalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = XcalcDoc.getChild("grid_window")
            # 1. Open attached document
            # 2. Place cursor in cell outside of subtotals range (e.g. B7)
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))
            # 3. Data → Subtotals
            with self.ui_test.execute_dialog_through_command(".uno:DataSubTotals") as xDialog:
                # 4. Group by: "- none -"
                xGroupBy = xDialog.getChild("group_by1")
                select_by_text(xGroupBy, "- none -")
                # 5. Press "OK" and watch LibreOffice crash.

            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getValue(), 1)
            self.assertEqual(get_cell_by_position(calc_doc, 0, 1, 1).getValue(), 2)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
