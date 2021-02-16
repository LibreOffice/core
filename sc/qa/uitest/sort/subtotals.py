# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.framework import UITestCase
from libreoffice.calc.document import get_column
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

class Subtotals(UITestCase):

    def test_tdf114720(self):
        calc_doc = self.ui_test.create_doc_in_start_center("calc")
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
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

        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(get_cell_by_position(document, 0, 0, 7).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 0, 8).getString(), "")

        # check cancel button
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.ui_test.close_doc()

    def test_tdf88792(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf88792.ods"))
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = XcalcDoc.getChild("grid_window")

        # go to cell A1
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
        # Select from the menu bar Data
        # Select option subtotal
        # Subtotal dialog displays
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        # Select group by: Category
        xGroupBy = xDialog.getChild("group_by")
        props = {"TEXT": "Category"}
        actionProps = mkPropertyValues(props)
        xGroupBy.executeAction("SELECT", actionProps)
        # Select calculate subtotals for the months -  selected by default
        # Select tab options
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "3")
        # select option include formats
        xformats = xDialog.getChild("formats")
        xformats.executeAction("CLICK", tuple())
        # apply with OK
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertEqual(get_cell_by_position(document, 0, 3, 5).getValue(), 28000)
        self.ui_test.close_doc()

    def test_tdf88735(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf88735.ods"))
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = XcalcDoc.getChild("grid_window")
        # 1 select all cells
        self.xUITest.executeCommand(".uno:SelectAll")#use uno command Menu Edit->Select All
        # 2 invoke sub-total menu and select none
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        xGroupBy = xDialog.getChild("group_by")
        props = {"TEXT": "- none -"}
        actionProps = mkPropertyValues(props)
        xGroupBy.executeAction("SELECT", actionProps)
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        # 2 invoke sort menu and... crash
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        xCancelBtn = xDialog.getChild("cancel")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        self.assertEqual(get_cell_by_position(document, 0, 0, 8).getString(), "z")
        self.assertEqual(get_cell_by_position(document, 0, 1, 8).getValue(), 8)
        self.ui_test.close_doc()

    def test_tdf56958(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf56958.ods"))
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = XcalcDoc.getChild("grid_window")
        # 1. Open the test file
        # 2. Data->Subtotals
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        # 3. Group by->Trans date
        xGroupBy = xDialog.getChild("group_by")
        props = {"TEXT": "Trans Date"}
        actionProps = mkPropertyValues(props)
        xGroupBy.executeAction("SELECT", actionProps)
        # 4. Tick 'Calculate subtotals for' -> Amount  (grid1)
        xCheckListMenu = xDialog.getChild("grid1")
        xTreeList = xCheckListMenu.getChild("columns")
        xFirstEntry = xTreeList.getChild("2")
        xFirstEntry.executeAction("CLICK", tuple())
        # 5. Click OK
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        # 6. Data->Subtotals
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        # 7. Group by->-none-
        xGroupBy = xDialog.getChild("group_by")
        props = {"TEXT": "- none -"}
        actionProps = mkPropertyValues(props)
        xGroupBy.executeAction("SELECT", actionProps)
        # 8. Untick 'Calculate subtotals for' -> Amount
        xCheckListMenu = xDialog.getChild("grid1")
        xTreeList = xCheckListMenu.getChild("columns")
        xFirstEntry = xTreeList.getChild("2")
        xFirstEntry.executeAction("CLICK", tuple())
        # 9. Click OK
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        # 10. Data->Sort
        self.ui_test.execute_dialog_through_command(".uno:DataSort")
        xDialog = self.xUITest.getTopFocusWindow()
        # 11. Sort key 1->Post Date.
        sortkey1 = xDialog.getChild("sortlb")
        props = {"TEXT": "Post Date"}
        actionProps = mkPropertyValues(props)
        sortkey1.executeAction("SELECT", actionProps)
        # 12. Sort key 2->-undefined-
        sortkey2 = xDialog.getChild("sortuserlb")
        props = {"TEXT": "- undefined -"}
        actionProps = mkPropertyValues(props)
        sortkey2.executeAction("SELECT", actionProps)
        # 13. Click OK
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)
        self.assertEqual(get_cell_by_position(document, 0, 2, 1).getValue(), -0.25)
        self.ui_test.close_doc()

    def test_tdf55734(self):
        calc_doc = self.ui_test.load_file(get_url_for_data_file("tdf55734.ods"))
        XcalcDoc = self.xUITest.getTopFocusWindow()
        document = self.ui_test.get_component()
        gridwin = XcalcDoc.getChild("grid_window")
        # 1. Open attached document
        # 2. Place cursor in cell outside of subtotals range (e.g. B7)
        gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B7"}))
        # 3. Data → Subtotals
        self.ui_test.execute_dialog_through_command(".uno:DataSubTotals")
        xDialog = self.xUITest.getTopFocusWindow()
        # 4. Group by: "- none -"
        xGroupBy = xDialog.getChild("group_by")
        props = {"TEXT": "- none -"}
        actionProps = mkPropertyValues(props)
        xGroupBy.executeAction("SELECT", actionProps)
        # 5. Press "OK" and watch LibreOffice crash.
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.assertEqual(get_cell_by_position(document, 0, 0, 1).getValue(), 1)
        self.assertEqual(get_cell_by_position(document, 0, 1, 1).getValue(), 2)
        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
